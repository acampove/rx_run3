'''
Script used to plot mass distributions
'''

import os
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
from ROOT                    import RDataFrame
from dmu.plotting.plotter_2d import Plotter2D
from dmu.logging.log_store   import LogStore
from rx_selection            import selection as sel
from rx_data.rdf_getter      import RDFGetter

log=LogStore.add_logger('rx_selection:plot_2d')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    mplhep.style.use('LHCb2')

    loglvl  : int
    q2bin   : str
    trigger : str
    config  : str
    sample  : str
    trigger : str
    ana_dir : str
# ---------------------------------
def _apply_selection(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    d_cut = cfg['selection']['cuts']
    if Data.q2bin is None:
        q2bin       = 'jpsi' # Need dummy cut for selection code
        d_cut['q2'] = '(1)'
    else:
        q2bin       = Data.q2bin

    sel.set_custom_selection(d_cut = d_cut)

    d_sel = sel.selection(trigger=Data.trigger, q2bin=q2bin, process=Data.sample)

    for cut_name, cut_expr in d_sel.items():
        log.debug(f'{cut_name:<20}{cut_expr}')
        rdf = rdf.Filter(cut_expr, cut_name)

    rep = rdf.Report()
    rep.Print()

    del cfg['selection']

    return rdf
# ---------------------------------
def _set_logs() -> None:
    LogStore.set_level('rx_selection:plot_2d'  , Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter2D', Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter'  , Data.loglvl)
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make 2D plots')
    parser.add_argument('-q', '--q2bin' , type=str, help='q2 bin, optional' , choices=['low', 'central', 'jpsi', 'psi2', 'high'])
    parser.add_argument('-c', '--config', type=str, help='Settings, i.e. mass_q2', required=True)
    parser.add_argument('-s', '--sample', type=str, help='Name of sample, can use wildcards', required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Name of trigger', required=True)
    parser.add_argument('-l', '--loglvl', type=int, help='Log level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.loglvl = args.loglvl
    Data.q2bin  = args.q2bin
    Data.config = args.config
# ---------------------------------
def _get_cfg() -> dict:
    config_path = files('rx_plotter_data').joinpath(f'2d/{Data.config}.yaml')
    config_path = str(config_path)

    with open(config_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    return _override_cfg(cfg)
# ---------------------------------
def _override_cfg(cfg : dict) -> dict:
    sample = Data.sample.replace('*', 'p')

    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{Data.ana_dir}/{plt_dir}/{Data.trigger}/{sample}'

    if Data.q2bin is None:
        return cfg

    for l_setting in cfg['plots_2d']:
        name = l_setting[3]
        l_setting[3] = f'{name}_{Data.q2bin}'

    return cfg
# ---------------------------------
def _initialize():
    _set_logs()
    Data.ana_dir = os.environ['ANADIR']
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    cfg = _get_cfg()
    if 'definitions' in cfg:
        log.warning('Adding custom definitions')
        RDFGetter.set_custom_columns(d_def = cfg['definitions'])
        del cfg['definitions']

    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()
    rdf = _apply_selection(rdf=rdf, cfg=cfg)

    ptr=Plotter2D(rdf=rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
if __name__ == 'main':
    main()
