'''
Script used to plot cutflows 
'''
import os
import glob
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
import dmu.generic.utilities as gut
from ROOT                    import RDataFrame, EnableImplicitMT
from dmu.plotting.plotter_1d import Plotter1D
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection

log=LogStore.add_logger('rx_selection:cutflow')
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

    mplhep.style.use('LHCb2')

    sample  : str
    chanel  : str
    substr  : str
    trigger : str
    q2_bin  : str
    config  : str
    plt_dir : str

    l_kind     = ['bdt', 'vetoes']
    l_ee_trees = ['brem_track_2', 'ecalo_bias']
    l_keep     = []
    l_col      = []
# ---------------------------------
def _initialize() -> None:
    if Data.nthreads > 1:
        EnableImplicitMT(Data.nthreads)

    dat_dir = os.environ['DATADIR']
    l_yaml  = glob.glob(f'{dat_dir}/samples/*.yaml')

    d_sample= {}
    log.info('Using paths:')
    for path in l_yaml:
        name = os.path.basename(path).replace('.yaml', '')
        if _skip_path(name):
            continue

        log.info(f'{name:<30}{path}')
        d_sample[name] = path

    RDFGetter.samples = d_sample
# ---------------------------------
def _skip_path(name : str) -> bool:
    # Do not pick up electron only (e.g. brem correction) trees
    # When working with muon samples
    if name in Data.l_ee_trees and 'MuMu' in Data.trigger:
        return True

    return False
# ---------------------------------
def _apply_definitions(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    if 'definitions' not in cfg:
        return rdf

    d_def = cfg['definitions']
    for name, expr in d_def.items():
        rdf = rdf.Define(name, expr)

    return rdf
# ---------------------------------
def _apply_selection(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    d_sel = selection.selection(project='RK', trigger=Data.trigger, q2bin=Data.q2_bin, process=Data.sample)

    if 'selection' in cfg:
        log.debug('Overriding selection')
        d_sel.update(cfg['selection'])

    for cut_name, cut_expr in d_sel.items():
        rdf = rdf.Filter(cut_expr, cut_name)

    rep = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()
    if nentries == 0:
        raise ValueError('No entries found after selection')

    return rdf
# ---------------------------------
@gut.timeit
def _get_rdf() -> RDataFrame:
    cfg = _get_cfg()
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_definitions(rdf, cfg)
    rdf = _apply_selection(rdf, cfg)

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
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to cutflow plots')
    parser.add_argument('-q', '--q2bin'  , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'])
    parser.add_argument('-s', '--sample' , type=str, help='Sample' , required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Trigger' , required=True)
    parser.add_argument('-c', '--config' , type=str, help='Configuration', choices=Data.l_kind)
    parser.add_argument('-x', '--substr' , type=str, help='Substring that must be contained in path, e.g. magup')
    args = parser.parse_args()

    Data.q2_bin = args.q2bin
    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.config = args.config
    Data.substr = args.substr
# ---------------------------------
def _get_cfg() -> dict:
    cfg_path = files('rx_plotter_data').joinpath(f'cutflow/{Data.config}.yaml')
    cfg_path = str(cfg_path)
    log.info(f'Picking configuration from: {cfg_path}')

    with open(cfg_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    plt_dir       = cfg['saving']['plt_dir']
    cfg['saving'] = {'plt_dir' : _get_out_dir(plt_dir) }

    if 'definitions' in cfg:
        del cfg['definitions']

    cfg = _add_title(cfg)

    return cfg
# ---------------------------------
def _add_title(cfg : dict) -> dict:
    d_plot = cfg['plots']

    title = f'{Data.sample}; {Data.trigger}; {Data.q2_bin}'
    for cfg_var in d_plot.values():
        cfg_var['title'] = title

    return cfg
# ---------------------------------
def _get_out_dir(plt_dir : str) -> str:
    sample  = Data.sample.replace('*', 'p')
    out_dir = f'{plt_dir}/{sample}_{Data.trigger}_{Data.q2_bin}'
    if Data.substr is not None:
        out_dir = f'{out_dir}/{Data.substr}'

    return out_dir
# ---------------------------------
def _get_inp() -> dict[str,RDataFrame]:
    cfg   = _get_cfg()
    rdf   = _get_rdf()

    if 'cutflow' not in cfg:
        return {'None' : rdf}

    d_cut = cfg['cutflow'][Data.q2_bin]
    d_rdf = {}
    log.info('Applying cutflow')
    for name, cut in d_cut.items():
        log.info(f'   {name}')
        rdf         = rdf.Filter(cut, name)
        d_rdf[name] = rdf

    return d_rdf
# ---------------------------------
def _plot(d_rdf : dict[str,RDataFrame]) -> None:
    cfg= _get_cfg()

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    d_rdf = _get_inp()
    _plot(d_rdf)
# ---------------------------------
if __name__ == 'main':
    main()
