'''
Script used to plot mass distributions
'''

import os
import argparse

import mplhep
from dmu.generic             import utilities as gut
from dmu.plotting.plotter_2d import Plotter2D
from dmu.logging.log_store   import LogStore
from rx_selection            import selection as sel
from rx_data.rdf_getter      import RDFGetter

log=LogStore.add_logger('rx_plots:plot_2d')
# ---------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    mplhep.style.use('LHCb2')

    nthreads: int = 1
    loglvl  : int = 20
    q2bin   : str = ''
    trigger : str = ''
    config  : str = ''
    sample  : str = ''
    trigger : str = ''
    ana_dir : str = os.environ['ANADIR']
# ---------------------------------
def _set_logs() -> None:
    LogStore.set_level('rx_plots:plot_2d'      , Data.loglvl)
    LogStore.set_level('rx_selection:selection', Data.loglvl)
    LogStore.set_level('rx_data:rdf_getter'    , Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter2D', Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter'  , Data.loglvl)
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make 2D plots')
    parser.add_argument('-q', '--q2bin'   , type=str, help='q2 bin, optional' , choices=['low', 'central', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-c', '--config'  , type=str, help='Settings, i.e. mass_q2', required=True)
    parser.add_argument('-s', '--sample'  , type=str, help='Name of sample, can use wildcards', required=True)
    parser.add_argument('-t', '--trigger' , type=str, help='Name of trigger', required=True)
    parser.add_argument('-l', '--loglvl'  , type=int, help='Log level', choices=[10, 20, 30], default=20)
    parser.add_argument('-n', '--nthreads', type=int, help='Number of threads', default=Data.nthreads)
    args = parser.parse_args()

    Data.sample   = args.sample
    Data.trigger  = args.trigger
    Data.loglvl   = args.loglvl
    Data.q2bin    = args.q2bin
    Data.config   = args.config
    Data.nthreads = args.nthreads
# ---------------------------------
def _override_output(cfg : dict) -> dict:
    '''
    Parameters
    ---------------
    cfg: Dictionary with plotting configuration

    Returns
    ---------------
    This method returns configuration with:

    - Plotting directory overriden
    - Name of output file overriden
    '''
    sample = Data.sample.replace('*', 'p')

    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{Data.ana_dir}/{plt_dir}/{Data.trigger}/{sample}'

    for l_setting in cfg['plots_2d']:
        name = l_setting[3]
        l_setting[3] = f'{name}_{Data.q2bin}'

    return cfg
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _set_logs()

    cfg   = gut.load_data(package='rx_plotter_data', fpath=f'2d/{Data.config}.yaml')
    cfg   = _override_output(cfg=cfg)
    d_cut = cfg['selection']['cuts']
    d_def = cfg['definitions']

    with RDFGetter.multithreading(nthreads=Data.nthreads), \
        RDFGetter.custom_columns(columns = d_def),\
        sel.custom_selection(d_sel=d_cut):

        del cfg['definitions']
        del cfg['selection']['cuts']

        gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
        rdf = gtr.get_rdf()

        rdf = sel.apply_full_selection(
            process= Data.sample,
            trigger= Data.trigger,
            q2bin  = Data.q2bin,
            rdf    = rdf)

        ptr=Plotter2D(rdf=rdf, cfg=cfg)
        ptr.run()
# ---------------------------------
if __name__ == '__main__':
    main()
