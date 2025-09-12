'''
Script used to plot overlays
'''
# pylint: disable=no-name-in-module, logging-fstring-interpolation
import os
import glob
import pprint
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
import dmu.generic.utilities as gut
from ROOT                    import RDataFrame, EnableImplicitMT # type: ignore
from dmu.plotting.plotter_1d import Plotter1D
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection as sel

log=LogStore.add_logger('rx_selection:cutflow')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    nthreads   = 1
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'
    d_reso     = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}
    data_dir   = os.environ['DATADIR']
    l_kind     = ['ecal_xy',
                  'brem',
                  'block_no_tail',
                  'npv',
                  'resolution',
                  'for_hlt',
                  'q2_cut',
                  'tail_cut',
                  'no_dtf_mass_shape']

    mplhep.style.use('LHCb2')

    chanel  : str
    substr  : str
    trigger : str
    q2_bin  : str
    cfg_dir : str
    brem    : int
    logl    : int

    l_col  = []
# ---------------------------------
def _initialize() -> None:
    if Data.nthreads > 1:
        EnableImplicitMT(Data.nthreads)

    cfg_dir = files('rx_plotter_data').joinpath('overlay')

    LogStore.set_level('rx_selection:cutflow', Data.logl)

    Data.cfg_dir = cfg_dir
    log.info(f'Picking configuration from: {Data.cfg_dir}')

    l_yaml  = glob.glob(f'{Data.data_dir}/samples/*.yaml')

    d_sample= { os.path.basename(path).replace('.yaml', '') : path for path in l_yaml }
    log.info('Using paths:')
    pprint.pprint(d_sample)
    RDFGetter.samples = d_sample
# ---------------------------------
def _apply_definitions(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    if 'definitions' not in cfg:
        return rdf

    d_def = cfg['definitions']
    for name, expr in d_def.items():
        rdf = rdf.Define(name, expr)

    return rdf
# ---------------------------------
def _filter_by_brem(rdf : RDataFrame) -> RDataFrame:
    if Data.brem is None:
        return rdf

    brem_cut = f'nbrem == {Data.brem}' if Data.brem in [0, 1] else f'nbrem >= {Data.brem}'
    rdf = rdf.Filter(brem_cut, 'nbrem')

    return rdf
# ---------------------------------
@gut.timeit
def _get_rdf() -> RDataFrame:
    cfg = _get_cfg()
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_definitions(rdf, cfg)

    d_sel = sel.selection(trigger=Data.trigger, q2bin=Data.q2_bin, process=Data.sample)

    if 'selection' in cfg:
        d_cut = cfg['selection']
        d_sel.update(d_cut)

    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
        rdf = rdf.Filter(cut_value, cut_name)

    rdf = _filter_by_brem(rdf)

    if log.getEffectiveLevel() == 10:
        rep = rdf.Report()
        rep.Print()

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
    parser = argparse.ArgumentParser(description='Script used to make generic plots')
    parser.add_argument('-q', '--q2bin'  , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-s', '--sample' , type=str, help='Sample' , required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Trigger' , required=True, choices=[Data.trigger_mm, Data.trigger_ee])
    parser.add_argument('-c', '--config' , type=str, help='Configuration', required=True)
    parser.add_argument('-x', '--substr' , type=str, help='Substring that must be contained in path, e.g. magup')
    parser.add_argument('-b', '--brem'   , type=int, help='Brem category', choices=[0, 1, 2])
    parser.add_argument('-l', '--logl'   , type=int, help='Log level'    , choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.q2_bin = args.q2bin
    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.config = args.config
    Data.substr = args.substr
    Data.brem   = args.brem
    Data.logl   = args.logl
# ---------------------------------
def _get_cfg() -> dict:
    cfg_path= f'{Data.cfg_dir}/{Data.config}.yaml'
    cfg_path= str(cfg_path)

    with open(cfg_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    cfg['saving'] = {'plt_dir' : _get_out_dir() }

    return cfg
# ---------------------------------
def _get_out_dir() -> str:
    if Data.brem is None:
        brem_name = 'all'
    else:
        brem_name = f'{Data.brem:03}'

    sample  = Data.sample.replace('*', 'p')
    out_dir = f'plots/{Data.config}/{sample}/{Data.trigger}/{Data.q2_bin}/{brem_name}'
    if Data.substr is not None:
        out_dir = f'{out_dir}/{Data.substr}'

    return out_dir
# ---------------------------------
def _get_inp() -> dict[str,RDataFrame]:
    cfg   = _get_cfg()
    rdf_in= _get_rdf()

    d_cut = cfg['overlay']
    d_rdf = {}
    log.info('Applying overlay')
    for name, cut in d_cut.items():
        log.info(f'   {name:<20}{cut}')
        rdf = rdf_in.Filter(cut, name)
        d_rdf[name] = rdf

    return d_rdf
# ---------------------------------
def _plot(d_rdf : dict[str,RDataFrame]) -> None:
    cfg= _get_cfg()
    if 'definitions' in cfg:
        del cfg['definitions']

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
if __name__ == '__main__':
    main()
