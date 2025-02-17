'''
Script used to plot mass distributions
'''
import os
import glob
import pprint
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
import dmu.generic.utilities as gut
from ROOT                    import RDataFrame, EnableImplicitMT
from dmu.plotting.plotter_1d import Plotter1D
from dmu.logging.log_store   import LogStore
from dmu.generic             import version_management as vmn
from rx_data.rdf_getter      import RDFGetter
from rx_selection.selection  import load_selection_config

log=LogStore.add_logger('rx_selection:plot_1d')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    nthreads   = 13
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'
    d_reso     = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}
    data_dir   = os.environ['DATADIR']

    mplhep.style.use('LHCb1')

    chanel  : str
    trigger : str
    q2_bin  : str
    q2_cut  : str
    version : str
    cfg_dir : str

    l_keep = []
# ---------------------------------
def _initialize() -> None:
    if Data.nthreads > 1:
        EnableImplicitMT(Data.nthreads)

    gut.TIMER_ON=True

    cfg_dir = files('rx_plotter_data').joinpath('')
    if Data.version is None:
        cfg_dir = vmn.get_last_version(dir_path=cfg_dir, version_only=False)
    else:
        cfg_dir = f'{cfg_dir}/{Data.version}'

    Data.cfg_dir = cfg_dir
    log.info(f'Picking configuration from: {Data.cfg_dir}')

    l_yaml  = glob.glob(f'{Data.data_dir}/samples/*.yaml')

    d_sample= { os.path.basename(path).replace('.yaml', '') : path for path in l_yaml }
    log.info('Using paths:')
    pprint.pprint(d_sample)
    RDFGetter.samples = d_sample
# ---------------------------------
@gut.timeit
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger=Data.trigger)
    rdf = gtr.get_rdf()
    Data.l_keep.append(rdf)

    return rdf
# ---------------------------------
@gut.timeit
def _get_bdt_cutflow_rdf(rdf : RDataFrame) -> dict[str,RDataFrame]:
    d_rdf = {}
    for cmb in [0.2, 0.4, 0.6, 0.8]:
        rdf = rdf.Filter(f'mva.mva_cmb > {cmb}')
        d_rdf [f'$MVA_{{cmb}}$ > {cmb}'] = rdf

    for prc in [0.2, 0.4, 0.6]:
        rdf = rdf.Filter(f'mva.mva_prc > {prc}')
        d_rdf [f'$MVA_{{prc}}$ > {prc}'] = rdf

    return d_rdf
# ---------------------------------
@gut.timeit
def _q2cut_from_q2bin(q2bin : str) -> str:
    cfg = load_selection_config()
    cut = cfg['q2_common'][q2bin]

    return cut
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make plots')
    parser.add_argument('-q', '--q2bin'  , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-c', '--chanel' , type=str, help='Channel', choices=['ee', 'mm'], required=True)
    parser.add_argument('-v', '--version', type=str, help='Version of inputs, will use latest if not set')
    args = parser.parse_args()

    Data.q2_bin = args.q2bin
    Data.q2_cut = _q2cut_from_q2bin(args.q2bin)
    Data.trigger= Data.trigger_mm if args.chanel == 'mm' else Data.trigger_ee
    Data.chanel = args.chanel
    Data.version= args.version
# ---------------------------------
def _get_cfg(kind : str) -> dict:
    cfg_path= f'{Data.cfg_dir}/bdt_q2_mass.yaml'
    cfg_path= str(cfg_path)

    with open(cfg_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    cfg = _override_cfg(cfg)

    if kind != 'last':
        return cfg

    d_plt        = cfg['plots']
    d_mas        = d_plt['B_M']
    if Data.chanel == 'mm':
        [_, maxx, nbins] = d_mas['binning']
        d_mas['binning'] = [5180, maxx, nbins]

    name         = d_mas['name']
    d_mas['name']= f'{name}_last'
    cfg['plots'] = {'B_M' : d_mas}

    return cfg
# ---------------------------------
def _add_reso_q2(cfg : dict) -> dict:
    d_mass              = cfg['plots']['B_M']
    d_mass              = dict(d_mass)
    d_mass['labels'][0] = r'M${}_{DTF}(B^+)$'
    d_mass['name'  ]    = f'DTF_mass_{Data.q2_bin}'

    reso_mass = Data.d_reso[Data.q2_bin]
    cfg['plots'][reso_mass] = d_mass

    return cfg
# ---------------------------------
def _get_cuts(cfg : dict) -> dict:
    if Data.q2_bin not in cfg['selection']:
        return {'q2' : Data.q2_cut}

    d_cut       = cfg['selection'][Data.q2_bin]
    d_cut['q2'] = Data.q2_bin

    return d_cut
# ---------------------------------
def _override_cfg(cfg : dict) -> dict:
    plt_dir                    = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{plt_dir}/{Data.trigger}'
    cfg['selection']['cuts']   = _get_cuts(cfg)
    cfg['style']['skip_lines'] = Data.chanel == 'mm'

    if Data.q2_bin in Data.d_reso:
        cfg = _add_reso_q2(cfg)

    for d_plot in cfg['plots'].values():
        if 'title' not in d_plot:
            d_plot['title'] = ''

        title = d_plot['title']
        title = f'{Data.q2_bin} $q^2$ {title}'
        d_plot['title'] = title

        name           = d_plot['name']
        d_plot['name'] = f'{Data.q2_bin}_{name}'

    return cfg
# ---------------------------------
@gut.timeit
def _plot(kind : str, d_rdf : dict[str,RDataFrame]) -> None:
    cfg   = _get_cfg(kind)

    if kind == 'last':
        key, val = d_rdf.popitem()
        d_rdf    = {key : val}

        cfg['stats'] = {'nentries' : '\nEntries: {}'}

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()

    _plot_paper_mass(d_rdf, cfg)
# ---------------------------------
@gut.timeit
def _plot_paper_mass(d_rdf, cfg):
    l_var = list(cfg['plots'])
    for var in l_var:
        if var == 'B_M':
            continue

        del cfg['plots'][var]

    if Data.q2_bin == 'high' and Data.chanel == 'ee':
        cfg['plots']['B_M']['binning'] = [4500, 6000, 60]
        cfg['plots']['B_M']['name']    = f'{Data.q2_bin}_bmass_paper'

        ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
        ptr.run()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    rdf   = _get_rdf()
    d_rdf = _get_bdt_cutflow_rdf(rdf)

    _plot(kind='cutflow', d_rdf = d_rdf)
    _plot(kind='last'   , d_rdf = d_rdf)
# ---------------------------------
if __name__ == 'main':
    main()
