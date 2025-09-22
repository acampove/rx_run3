'''
Script used to compare variables in the same dataframe
'''
# pylint: disable=no-name-in-module, logging-fstring-interpolation, line-too-long

import os
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
import dmu.generic.utilities as gut
from ROOT                    import RDataFrame # type: ignore
from dmu.plotting.plotter_1d import Plotter1D
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection as sel

log=LogStore.add_logger('rx_selection:compare')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    trigger_bukee = 'Hlt2RD_BuToKpEE_MVA'
    trigger_bukmm = 'Hlt2RD_BuToKpMuMu_MVA'

    trigger_bdkee = 'Hlt2RD_B0ToKpPimEE_MVA'
    trigger_bdkmm = 'Hlt2RD_B0ToKpPimMuMu_MVA'

    l_trigger_bu  = [trigger_bukmm, trigger_bukee, f'{trigger_bukee}_ext', f'{trigger_bukee}_noPID']
    l_trigger_bd  = [trigger_bdkmm, trigger_bdkee, f'{trigger_bdkee}_ext', f'{trigger_bdkee}_noPID']
    l_trigger     = l_trigger_bd + l_trigger_bu

    d_reso        = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}
    ana_dir       = os.environ['ANADIR']

    q2_bin  : str
    sample  : str
    trigger : str
    config  : str
    substr  : str
    brem    : int
    block   : int
    nomva   : bool
    chanel  : str
    cfg     : dict
    nthread : int = 1

    l_col  = []
# ---------------------------------
def _initialize() -> None:
    '''
    This should run at the beginning
    '''
    mplhep.style.use('LHCb2')

    Data.cfg = _get_cfg()
# ---------------------------------
def _get_cfg() -> dict:
    cfg_dir = files('rx_plotter_data').joinpath('compare')
    cfg_path= f'{cfg_dir}/{Data.config}.yaml'
    cfg_path= str(cfg_path)

    log.info(f'Picking configuration from: {cfg_path}')
    with open(cfg_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir'] = _get_out_dir(plt_dir)

    for d_setting in cfg['plots'].values():
        brem, block        = _get_brem_block()
        d_setting['title'] = f'Brem {brem}; Block {block}; {Data.sample}'

    return cfg
# ---------------------------------
def _get_brem_block() -> tuple[str,str]:
    brem = 'all' if Data.brem == -1 else str(Data.brem)
    block= 'all' if Data.block== -1 else str(Data.block)

    return brem, block
# ---------------------------------
def _apply_definitions(rdf : RDataFrame) -> RDataFrame:
    if 'definitions' not in Data.cfg:
        return rdf

    d_def = Data.cfg['definitions']
    for name, expr in d_def.items():
        rdf = rdf.Define(name, expr)

    del Data.cfg['definitions']

    return rdf
# ---------------------------------
def _check_entries(rdf : RDataFrame) -> None:
    nentries = rdf.Count().GetValue()

    if nentries > 0:
        return

    rep = rdf.Report()
    rep.Print()

    raise ValueError('Found zero entries in dataframe')
# ---------------------------------
def _update_with_brem(d_sel : dict[str,str]) -> dict[str,str]:
    if Data.brem == -1:
        log.info('Not filtering by brem')
        return d_sel

    if Data.brem == 12:
        d_sel['nbrem'] = 'nbrem == 1 || nbrem == 2'
    else:
        d_sel['nbrem'] = f'nbrem == {Data.brem}'

    return d_sel
# ---------------------------------
def _update_with_block(d_sel : dict[str,str]) -> dict[str,str]:
    if Data.block == -1:
        log.info('Not filtering by block')
        return d_sel

    d_sel['block'] = f'block == {Data.block}'

    return d_sel
# ---------------------------------
@gut.timeit
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf(per_file=False)
    rdf = _apply_definitions(rdf)
    d_sel = sel.selection(trigger=Data.trigger, q2bin=Data.q2_bin, process=Data.sample)

    if Data.nomva:
        log.info('Removing MVA requirement')
        d_sel['bdt'] = '(1)'

    if 'selection' in Data.cfg:
        d_cut = Data.cfg['selection']
        d_sel.update(d_cut)
        del Data.cfg['selection']

    d_sel = _update_with_brem(d_sel)
    d_sel = _update_with_block(d_sel)

    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
        rdf = rdf.Filter(cut_value, cut_name)

    return rdf
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make comparison plots between distributions in the same dataframe')
    parser.add_argument('-q', '--q2bin'  , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-s', '--sample' , type=str, help='Sample' , required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Trigger' , required=True, choices=Data.l_trigger)
    parser.add_argument('-c', '--config' , type=str, help='Configuration', required=True)
    parser.add_argument('-x', '--substr' , type=str, help='Substring that must be contained in path, e.g. magup')
    parser.add_argument('-b', '--brem'   , type=int, help='Brem category, 12 = 1 or 2, -1 = all' , choices=[-1, 0, 1, 2, 12], required=True)
    parser.add_argument('-n', '--nthread', type=int, help='Number of threads' , default=Data.nthread)
    parser.add_argument('-B', '--block'  , type=int, help='Block to which data belongs, -1 will put all the data together', choices=[-1, 0, 1, 2, 3, 4, 5, 6, 7, 8], required=True)
    parser.add_argument('-r', '--nomva'  ,           help='If used, it will remove the MVA requirement', action='store_true')

    args = parser.parse_args()

    Data.q2_bin = args.q2bin
    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.config = args.config
    Data.substr = args.substr
    Data.brem   = args.brem
    Data.block  = args.block
    Data.nomva  = args.nomva
    Data.nthread= args.nthread
# ---------------------------------
def _get_out_dir(plt_dir : str) -> str:
    brem, block = _get_brem_block()

    sample  = Data.sample.replace('*', 'p')
    out_dir = f'{Data.ana_dir}/{plt_dir}/{sample}/{Data.trigger}/{Data.q2_bin}/{brem}_{block}'
    if Data.nomva:
        out_dir = f'{out_dir}/drop_mva'
    else:
        out_dir = f'{out_dir}/with_mva'

    if Data.substr is not None:
        out_dir = f'{out_dir}/{Data.substr}'

    return out_dir
# ---------------------------------
def _rdf_from_def(rdf : RDataFrame, d_def : dict) -> RDataFrame:
    d_var = d_def['vars']
    for name, expr in d_var.items():
        rdf = rdf.Define(name, expr)

    if 'cuts' not in d_def:
        return rdf

    d_cut = d_def['cuts']
    for cut_name, cut_expr in d_cut.items():
        rdf = rdf.Filter(cut_expr, cut_name)

    return rdf
# ---------------------------------
def _get_inp() -> dict[str,RDataFrame]:
    rdf_in = _get_rdf()

    d_rdf = {}
    d_cmp = Data.cfg['comparison']
    for kind, d_def in d_cmp.items():
        rdf = _rdf_from_def(rdf_in, d_def)
        _check_entries(rdf)
        d_rdf[kind] = rdf

    del Data.cfg['comparison']

    return d_rdf
# ---------------------------------
def _plot(d_rdf : dict[str,RDataFrame]) -> None:
    ptr=Plotter1D(d_rdf=d_rdf, cfg=Data.cfg)
    ptr.run()
# ---------------------------------
def main() -> None:
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()
    with RDFGetter.multithreading(nthreads=Data.nthread):
        d_rdf = _get_inp()
        _plot(d_rdf)
# ---------------------------------
if __name__ == '__main__':
    main()
