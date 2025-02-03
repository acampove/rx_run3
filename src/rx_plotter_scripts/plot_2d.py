'''
Script used to plot mass distributions
'''
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
from ROOT                    import RDataFrame, EnableImplicitMT
from dmu.plotting.plotter_2d import Plotter2D
from dmu.logging.log_store   import LogStore
from rx_selection.selection  import load_selection_config
from rx_data.rdf_getter      import RDFGetter

log=LogStore.add_logger('rx_selection:plot_2d')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    nthreads   = 13
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'

    mplhep.style.use('LHCb1')

    RDFGetter.samples_dir = '/home/acampove/Data/RX_run3/NO_q2_bdt_mass_Q2_central_VR_v1'

    loglvl  : int
    q2bin   : str
    chanel  : str
    trigger : str
    settn   : str
# ---------------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger=Data.trigger)
    rdf = gtr.get_rdf()

    return rdf
# ---------------------------------
def _set_logs() -> None:
    LogStore.set_level('rx_selection:plot_2d'  , Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter2D', Data.loglvl)
    LogStore.set_level('dmu:plotting:Plotter'  , Data.loglvl)
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make 2D plots')
    parser.add_argument('-c', '--chanel', type=str, help='Channel', choices=['ee', 'mm'], required=True)
    parser.add_argument('-q', '--q2bin' , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'])
    parser.add_argument('-s', '--settn' , type=str, help='Settings, i.e. bdt_q2', choices=['bdt_q2', 'cmb_prc'], required=True)
    parser.add_argument('-l', '--loglvl', type=int, help='Log level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.trigger= Data.trigger_mm if args.chanel == 'mm' else Data.trigger_ee
    Data.chanel = args.chanel
    Data.loglvl = args.loglvl
    Data.q2bin  = args.q2bin
    Data.settn  = args.settn
# ---------------------------------
def _get_cfg() -> dict:
    config_path = files('rx_plotter_data').joinpath(f'{Data.settn}.yaml')
    config_path = str(config_path)

    with open(config_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    return _override_cfg(cfg)
# ---------------------------------
def _override_cfg(cfg : dict) -> dict:
    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{plt_dir}/{Data.trigger}'

    for d_plot in cfg['axes'].values():
        d_plot['title'] = f'Channel: {Data.chanel}'

    return cfg
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _set_logs()
    EnableImplicitMT(Data.nthreads)

    rdf   = _get_rdf()
    cfg   = _get_cfg()

    ptr=Plotter2D(rdf=rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
if __name__ == 'main':
    main()
