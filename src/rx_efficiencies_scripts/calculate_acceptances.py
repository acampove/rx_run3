#!/usr/bin/env python3

import os
import glob
import ROOT
import pprint
import pandas            as pnd
import argparse
import utils_noroot      as utnr
import matplotlib.pyplot as plt

from version_management    import get_last_version 
from importlib.resources   import files
from log_store             import log_store
from acceptance_calculator import calculator

log = log_store.add_logger('calculate_acceptance')
#----------------------------------
class data:
    out_dir = None
    l_energy= ['7TeV', '8TeV', '13TeV'] 
#----------------------------------
def get_tex():
    d_tex                = dict()
    d_tex['bdkskpiee'  ] =  r'$B_d\to K^{*0}(\to K^+\pi^-)e^+e^-$'
    d_tex['bpkskpiee'  ] =  r'$B^+\to K^{*+}(\to K^+\pi^0)e^+e^-$'
    d_tex['B2Kee'      ] =  r'$B^+\to K^+e^+e^-$'
    d_tex['bsphiee'    ] =  r'$B_s\to \phi(1020)e^+e^-$'
    d_tex['bpk2kpipiee'] =  r'$B^+\to K_2(1430)^+(\to X \to K^+\pi^+\pi^-)e^+e^-$'
    d_tex['bpk1kpipiee'] =  r'$B^+\to K_1(1270)^+(\to K^+\pi^+\pi^-)e^+e^-$'

    return d_tex
#----------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to dump JSON file with acceptances')
    parser.add_argument('-e', '--energy', nargs='+', help='Center of mass energy', default=data.l_energy, choices=data.l_energy)
    parser.add_argument('-v', '--version', type=str, help='Version for directory containing JSON files with acceptances', required=True)
    args = parser.parse_args()

    data.l_energy = args.energy
    data.out_dir  = get_out_dir(args.version)
#----------------------------------
def get_out_dir(version):
    out_dir = files('prec_acceptances_data').joinpath(version)

    try:
        os.makedirs(out_dir, exist_ok=True)
    except:
        log.error(f'Cannot make: {out_dir}')
        raise

    return out_dir
#---------------------------------
def get_id(path):
    filename   = os.path.basename(path)
    identifier = filename.replace('_tree.root', '')

    return identifier
#----------------------------------
def get_paths(energy):
    dat_dir = os.environ['DATDIR']
    version = get_last_version(dir_path=f'{dat_dir}/acceptance', version_only=True)
    root_wc = f'{dat_dir}/acceptance/{version}/{energy}/*_tree.root'

    l_path  = glob.glob(root_wc)
    if len(l_path) == 0:
        log.error(f'No files found in: {root_wc}')
        raise FileNotFoundError
    else:
        log.info(f'Picking up files from: {root_wc}')

    d_path  = { get_id(path) : path for path in l_path }

    log.info('Found paths:')
    pprint.pprint(d_path)

    return d_path
#----------------------------------
def get_acceptances(decay, path, energy):
    rdf = ROOT.RDataFrame('DecayTree', path)
    nent= rdf.Count().GetValue()

    obj = calculator(rdf)
    obj.plot_dir     = f'{data.out_dir}/plots_{energy}/{decay}'
    acc_phy, acc_lhc = obj.get_acceptances()

    return acc_phy, acc_lhc 
#----------------------------------
def load_df(energy):
    jsn_path = f'{data.out_dir}/acceptances_{energy}.json'
    if not os.path.isfile(jsn_path):
        return

    df = pnd.read_json(jsn_path)

    return df
#----------------------------------
def get_df(energy):
    df = load_df(energy)
    if df is not None:
        return df

    d_tex  = get_tex()
    d_path = get_paths(energy)
    d_out  = {'Process' : [], 'Physical' : [], 'LHCb' : []}
    for decay, path in d_path.items():
        log.debug(f'Checking {decay}')
        acc_phy, acc_lhc = get_acceptances(decay, path, energy)

        tex_decay = d_tex[decay]

        d_out['Process' ].append(tex_decay)
        d_out['Physical'].append(acc_phy)
        d_out['LHCb'    ].append(acc_lhc)

    df = pnd.DataFrame(d_out)

    return df
#----------------------------------
def save_tables(df, energy):
    tex_path = f'{data.out_dir}/acceptances_{energy}.tex'
    log.info(f'Saving to: {tex_path}')
    utnr.df_to_tex(df, tex_path, hide_index=True, d_format={'Process' : '{}', 'Physical' : '{:.3f}', 'LHCb' : '{:.3f}'}, caption=None)

    jsn_path = f'{data.out_dir}/acceptances_{energy}.json'
    log.info(f'Saving to: {jsn_path}')
    df.to_json(jsn_path, indent=4)
#----------------------------------
def plot_acceptance(df, kind):
    fig, ax = plt.subplots(figsize=(8,6))
    for process, df_p in df.groupby('Process'):
        df_p.plot(x='Energy', y=kind, ax=ax, label=process)

    plt.ylim(0.0, 0.20)
    plt.grid()
    plot_path = f'{data.out_dir}/acceptances_{kind}.png'
    log.info(f'Saving to: {plot_path}')
    plt.savefig(plot_path)
    plt.close('all')
#----------------------------------
def main():
    get_args()
    l_df = []
    for energy in data.l_energy:
        df=get_df(energy)
        save_tables(df, energy)

        df['Energy'] = energy
        l_df.append(df)

    df = pnd.concat(l_df, axis=0)

    plot_acceptance(df, 'LHCb')
    plot_acceptance(df, 'Physical')
#----------------------------------
if __name__ == '__main__':
    main()

