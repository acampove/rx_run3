'''
Script used to plot mass distributions
'''

import os
import glob
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

    mplhep.style.use('LHCb2')

    loglvl  : int
    q2bin   : str
    chanel  : str
    trigger : str
    config  : str
    sample  : str
    trigger : str
# ---------------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
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
    parser.add_argument('-C', '--chanel', type=str, help='Channel', choices=['ee', 'mm'], required=True)
    parser.add_argument('-q', '--q2bin' , type=str, help='q2 bin, optional' , choices=['low', 'central', 'jpsi', 'psi2', 'high'])
    parser.add_argument('-c', '--config', type=str, help='Settings, i.e. mass_q2', required=True)
    parser.add_argument('-s', '--sample', type=str, help='Name of sample, can use wildcards', required=True)
    parser.add_argument('-t', '--trigger', type=str, help='Name of trigger', required=True)
    parser.add_argument('-l', '--loglvl', type=int, help='Log level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.chanel = args.chanel
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
    plt_dir = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{plt_dir}/{Data.trigger}'

    if Data.q2bin is None:
        return cfg

    cfg_sel = load_selection_config()
    cut     = cfg_sel['q2_common'][Data.q2bin]
    cfg['selection'] = {'cuts' : {'q2' : cut}}

    for l_setting in cfg['plots_2d']:
        name = l_setting[3]
        l_setting[3] = f'{name}_{Data.q2bin}'

    return cfg
# ---------------------------------
def _set_samples() -> None:
    data_dir  = os.environ['DATADIR']
    sample_wc = f'{data_dir}/samples/*.yaml'
    l_path    = glob.glob(sample_wc)
    d_sample  = {}
    for path in l_path:
        file_name = os.path.basename(path)
        name      = file_name.replace('.yaml', '')

        if Data.chanel == 'mm' and name in ['brem_track_2', 'ecalo_bias']:
            continue

        d_sample[name] = path

    RDFGetter.samples = d_sample
# ---------------------------------
def _initialize():
    _set_logs()
    _set_samples()
    EnableImplicitMT(Data.nthreads)
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    rdf = _get_rdf()
    cfg = _get_cfg()

    ptr=Plotter2D(rdf=rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
if __name__ == 'main':
    main()
