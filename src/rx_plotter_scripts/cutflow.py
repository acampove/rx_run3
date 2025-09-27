'''
Script used to plot cutflows
'''
import os
import argparse

import mplhep
import dmu.generic.utilities as gut
from ROOT                    import RDataFrame # type: ignore
from dmu.plotting.plotter_1d import Plotter1D
from dmu.logging.log_store   import LogStore
from dmu.generic             import naming
from omegaconf               import DictConfig
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection

log=LogStore.add_logger('rx_selection:cutflow')
# ---------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'
    d_reso     = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}
    ana_dir    = os.environ['ANADIR']

    mplhep.style.use('LHCb2')

    sample  : str
    chanel  : str
    substr  : str
    trigger : str
    q2_bin  : str
    config  : str
    plt_dir : str
    nthreads: int
    cfg     : dict
    d_def   : dict[str,str] = {}

    l_ee_trees = ['brem_track_2', 'ecalo_bias']
    l_keep     = []
    l_col      = []
# ---------------------------------
def _apply_definitions(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    if 'definitions' not in cfg:
        return rdf

    d_def = cfg['definitions']
    for name, expr in d_def.items():
        name = naming.clean_special_characters(name=name)
        rdf  = rdf.Define(name, expr)

    return rdf
# ---------------------------------
def _apply_selection(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    d_sel = selection.selection(trigger=Data.trigger, q2bin=Data.q2_bin, process=Data.sample)

    if 'selection' in cfg:
        log.debug('Overriding selection')
        d_sel.update(cfg['selection'])

    log.info(40 * '-')
    log.info('Applying selection')
    log.info(40 * '-')
    for cut_name, cut_expr in d_sel.items():
        log.info(f'{cut_name:<20}{cut_expr}')
        name = naming.clean_special_characters(name=cut_name)
        rdf  = rdf.Filter(cut_expr, name)

    rep = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()
    if nentries == 0:
        raise ValueError('No entries found after selection')

    return rdf
# ---------------------------------
@gut.timeit
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_definitions(rdf, Data.cfg)
    rdf = _apply_selection(rdf, Data.cfg)

    return rdf
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to cutflow plots')
    parser.add_argument('-q', '--q2bin'  , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high', 'high_dtf'])
    parser.add_argument('-s', '--sample' , type=str, help='Sample' , required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Trigger' , required=True)
    parser.add_argument('-c', '--config' , type=str, help='Configuration', required=True)
    parser.add_argument('-x', '--substr' , type=str, help='Substring that must be contained in path, e.g. magup')
    parser.add_argument('-n', '--nthreads',type=int, help='Number of threads, if using multithreading', default=1)
    args = parser.parse_args()

    Data.q2_bin   = args.q2bin
    Data.sample   = args.sample
    Data.trigger  = args.trigger
    Data.config   = args.config
    Data.substr   = args.substr
    Data.nthreads = args.nthreads
# ---------------------------------
def _get_cfg() -> dict:
    cfg           = gut.load_data(package='rx_plotter_data', fpath=f'cutflow/{Data.config}.yaml')
    plt_dir       = cfg['saving']['plt_dir']
    cfg['saving'] = {'plt_dir' : _get_out_dir(plt_dir) }

    if 'definitions' in cfg:
        Data.d_def = cfg['definitions']
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
    out_dir = f'{Data.ana_dir}/plots/{plt_dir}/{Data.sample}_{Data.trigger}_{Data.q2_bin}'
    if Data.substr is not None:
        out_dir = f'{out_dir}/{Data.substr}'

    return out_dir
# ---------------------------------
def _get_inp() -> dict[str,RDataFrame]:
    rdf   = _get_rdf()

    if 'cutflow' not in Data.cfg:
        return {'None' : rdf}

    d_cut = Data.cfg['cutflow'][Data.q2_bin]
    d_rdf = {}
    log.info('Applying cutflow')
    for name, cut in d_cut.items():
        log.info(f'{"":<4}{name:<15}{cut}')
        name        = naming.clean_special_characters(name=name)
        rdf         = rdf.Filter(cut, name)
        d_rdf[name] = rdf

    return d_rdf
# ---------------------------------
def _fix_ranges(cfg : dict) -> dict:
    '''
    Takes configuration and makes sure mass ranges make sense
    '''

    cfg_plt = cfg['plots']
    if 'B_M' in cfg_plt and 'MuMu' in Data.trigger:
        [_, _, nbins] = cfg_plt['B_M']['binning']
        cfg_plt['B_M']['binning'] = [5150, 5800, nbins]

    return cfg
# ---------------------------------
def _plot(d_rdf : dict[str,RDataFrame]) -> None:
    cfg= _get_cfg()
    cfg= _fix_ranges(cfg)

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
def _set_config(settings : DictConfig) -> None:
    '''
    Parameters
    -------------
    settings: Dictionary with settings needed when used as a module
    '''
    Data.q2_bin   = settings.q2bin
    Data.sample   = settings.sample
    Data.trigger  = settings.trigger
    Data.config   = settings.config
    Data.substr   = settings.substr
    Data.nthreads = settings.nthreads
# ---------------------------------
def main(settings : DictConfig|None = None):
    '''
    Script starts here
    '''
    if settings is None:
        _parse_args()
    else:
        _set_config(settings=settings)

    Data.cfg = _get_cfg()
    with RDFGetter.multithreading(nthreads=Data.nthreads),\
         RDFGetter.max_entries(value=-1),\
         RDFGetter.custom_columns(columns=Data.d_def):

        d_rdf = _get_inp()
        _plot(d_rdf)
# ---------------------------------
if __name__ == '__main__':
    main()
