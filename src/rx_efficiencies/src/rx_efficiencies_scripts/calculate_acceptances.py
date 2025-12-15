'''
Script used to calculate Geometric acceptances through AcceptanceCalculator and save them to JSON
'''

import os
import glob
import pprint
import argparse

import mplhep
import pandas                   as pnd
import matplotlib.pyplot        as plt

from typing                import Union
from pathlib               import Path
from ROOT                  import RDataFrame # type: ignore
from dmu.pdataframe        import utilities as put
from dmu                   import LogStore
from rx_common import Sample

from rx_efficiencies.acceptance_calculator import AcceptanceCalculator

log = LogStore.add_logger('rx_efficiencies:calculate_acceptance')
#----------------------------------
class Data:
    '''
    Data class
    '''
    ana_dir = Path(os.environ['ANADIR'])
    out_dir : Path 
    version : str
    l_energy= ['8TeV', '13TeV', '14TeV']

    plt.style.use(mplhep.style.LHCb2)
#----------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to dump JSON file with acceptances')
    parser.add_argument('-e', '--energy' , nargs='+', help='Center of mass energy', default=Data.l_energy, choices=Data.l_energy)
    parser.add_argument('-v', '--version', type=str, help='Version for directory containing JSON files with acceptances', required=True)
    args = parser.parse_args()

    Data.version  = args.version
    Data.l_energy = args.energy
    Data.out_dir  = Data.ana_dir / f'efficiencies/acceptances/{Data.version}'
    Data.out_dir.mkdir(parents = True, exist_ok=True)
#---------------------------------
def _get_id(path : Path) -> Sample:
    '''
    Parameters
    ------------------
    path: Path to ROOT file created with Rapidsim

    Returns
    ------------------
    nickname of decay, e.g. bpkpee
    '''
    filename   = path.name
    identifier = filename.replace('_tree.root', '')

    value = None
    for sample in Sample:
        if identifier == sample.name:
            value = sample
            break

    if value is None:
        raise ValueError(f'Cannot recognize {identifier} as a Sample')

    return value 
#----------------------------------
def _get_paths(energy : str):
    dat_dir = f'{Data.ana_dir}/Rapidsim'
    root_wc = f'{dat_dir}/*/*/*/*'

    l_org   = glob.glob(root_wc)
    l_path  = l_org
    l_path  = [ path for path in l_path if  '_tree.root' in path ]
    l_path  = [ path for path in l_path if f'/{energy}/' in path ]

    if len(l_path) == 0:
        for path in l_org:
            log.info(path)
        raise FileNotFoundError(f'No files found in: {root_wc} at {energy}')

    log.info(f'Picking up files from: {root_wc}')
    d_path  = { _get_id(path) : path for path in l_path }

    log.info('Found paths:')
    pprint.pprint(d_path)

    return d_path
#----------------------------------
def _get_acceptances(decay, path, energy):
    rdf = RDataFrame('DecayTree', path)
    obj = AcceptanceCalculator(rdf=rdf)
    obj.plot_dir     = f'{Data.out_dir}/plots_{energy}/{decay}'
    acc_phy, acc_lhc = obj.get_acceptances()

    return acc_phy, acc_lhc
#----------------------------------
def _load_df(energy : str) -> Union[None, pnd.DataFrame]:
    jsn_path = f'{Data.out_dir}/acceptances_{energy}.json'
    if not os.path.isfile(jsn_path):
        return None

    df = pnd.read_json(jsn_path)

    return df
#----------------------------------
def _get_df(energy : str) -> pnd.DataFrame:
    df = _load_df(energy)
    if df is not None:
        return df

    d_path = _get_paths(energy)
    d_out  = {'Process' : [], 'Physical' : [], 'LHCb' : []}
    for decay, path in d_path.items():
        log.debug(f'Checking {decay}')
        acc_phy, acc_lhc = _get_acceptances(decay, path, energy)

        tex_decay = DecayNames.tex_from_decay(decay)

        d_out['Process' ].append(tex_decay)
        d_out['Physical'].append(acc_phy)
        d_out['LHCb'    ].append(acc_lhc)

    df = pnd.DataFrame(d_out)

    return df
#----------------------------------
def _save_tables(df, energy):
    tex_path = f'{Data.out_dir}/acceptances_{energy}.tex'
    log.info(f'Saving to: {tex_path}')
    put.df_to_tex(df, tex_path, hide_index=True, d_format={'Process' : '{}', 'Physical' : '{:.3f}', 'LHCb' : '{:.3f}'})

    jsn_path = f'{Data.out_dir}/acceptances_{energy}.json'
    log.info(f'Saving to: {jsn_path}')
    df.to_json(jsn_path, indent=4)
#----------------------------------
def _plot_acceptance(df, kind):
    _, ax = plt.subplots(figsize=(8,6))
    for process, df_p in df.groupby('Process'):
        df_p.plot(x='Energy', y=kind, ax=ax, label=process, figsize=(12, 8))

    plt.ylim(0.0, 0.20)
    plt.grid()
    plt.title(kind)
    plt.ylabel('Acceptance')
    plt.xlabel('')
    plot_path = f'{Data.out_dir}/acceptances_{kind}.png'
    log.info(f'Saving to: {plot_path}')
    plt.savefig(plot_path)
    plt.close('all')
#----------------------------------
def main():
    '''
    Start here
    '''
    _get_args()
    l_df = []
    for energy in Data.l_energy:
        df=_get_df(energy)
        _save_tables(df, energy)

        df['Energy'] = energy
        l_df.append(df)

    df = pnd.concat(l_df, axis=0)

    _plot_acceptance(df, 'LHCb')
    _plot_acceptance(df, 'Physical')
#----------------------------------
if __name__ == '__main__':
    main()
