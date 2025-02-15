'''
Script in charge of training classifier
'''
# pylint: disable=import-error

import os
import re
import glob
import argparse

from importlib.resources import files
from dataclasses         import dataclass

import matplotlib.pyplot as plt
import mplhep
import yaml

from rx_data.rdf_getter    import RDFGetter
from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva

log = LogStore.add_logger('rx_classifier:train_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class meant to hold data to be shared
    '''
    root_regex  = r'\d{3}_\d{3}.root'
    cfg_dict    : dict
    cfg_name    : str
    version     : str
    q2bin       : str
    max_entries : int
#---------------------------------
def _override_version(path : str) -> str:
    if 'VERSION' not in path:
        raise ValueError(f'VERSION expected in: {path}')

    replacement = f'{Data.version}/{Data.q2bin}'

    return path.replace('VERSION', replacement)
#---------------------------------
def _reformat_config(cfg : dict) -> dict:
    path = cfg['saving']['path']
    cfg['saving']['path']      = _override_version(path)

    path = cfg['plotting']['val_dir']
    cfg['plotting']['val_dir'] = _override_version(path)

    path = cfg['plotting']['features']['saving']['plt_dir']
    cfg['plotting']['features']['saving']['plt_dir'] = _override_version(path)

    cfg['training']['features'] = cfg['features'][Data.q2bin]

    return cfg
#---------------------------------
def _reformat_hyperparameters(cfg : dict) -> dict:
    d_common_hyper = cfg['hyper']
    d_q2hyper      = cfg['training']['hyper'][Data.q2bin]

    d_common_hyper.update(d_q2hyper)

    cfg['training']['hyper'] = d_common_hyper

    return cfg
#---------------------------------
def _load_config():
    '''
    Will load YAML file config
    '''

    cfg_path = files('rx_classifier_data').joinpath(f'{Data.version}/{Data.cfg_name}.yaml')
    cfg_path = str(cfg_path)
    if not os.path.isfile(cfg_path):
        raise FileNotFoundError(f'Could not find: {cfg_path}')

    with open(cfg_path, encoding='utf-8') as ifile:
        cfg_dict = yaml.safe_load(ifile)
        cfg_dict = _reformat_config(cfg_dict)
        cfg_dict = _reformat_hyperparameters(cfg_dict)

    Data.cfg_dict = cfg_dict
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Used to train classifier based on config file')
    parser.add_argument('-v', '--version'    , type=str, help='Version of config files', required=True)
    parser.add_argument('-c', '--cfg_name'   , type=str, help='Kind of config file'    , required=True)
    parser.add_argument('-q', '--q2bin'      , type=str, help='q2bin'                  , required=True, choices=['low', 'central', 'jpsi', 'psi2S', 'high'])
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=20, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    args = parser.parse_args()

    Data.version     = args.version
    Data.cfg_name    = args.cfg_name
    Data.q2bin       = args.q2bin
    Data.max_entries = args.max_entries

    LogStore.set_level('rx_classifier:train_classifier', args.log_level)
    LogStore.set_level('dmu:ml:train_mva'              , args.log_level)
#---------------------------------
def _is_ntuple_path(path : str) -> bool:
    file_name = os.path.basename(path)
    mtch = re.match(Data.root_regex, file_name)

    return bool(mtch)
#---------------------------------
def _file_paths_from_wc(file_wc : str) -> list[str]:
    l_path = glob.glob(file_wc)
    if len(l_path) == 0:
        raise FileNotFoundError(f'Cannot find any file in: {file_wc}')

    l_path = [ path for path in l_path if _is_ntuple_path(path) ]
    if len(l_path) == 0:
        raise FileNotFoundError(f'Cannot find any ROOT file with ntuples in: {file_wc}')

    nfile = len(l_path)
    log.info(f'Found {nfile} files in: {file_wc}')

    return l_path
#---------------------------------
def _get_rdf(kind=None):
    '''
    Will load and return ROOT dataframe

    Parameters
    ---------------------
    kind (str): kind of dataset to find in config input section
    '''
    sample  = Data.cfg_dict['dataset']['samples'][kind]['sample']
    trigger = Data.cfg_dict['dataset']['samples'][kind]['trigger']
    l_col   = Data.cfg_dict['dataset']['columns']

    RDFGetter.samples = Data.cfg_dict['dataset']['paths']
    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf(columns=l_col)

    rdf = _define_columns(rdf)
    rdf = _apply_selection(rdf, kind)

    if Data.max_entries > 0:
        log.warning(f'Limiting {kind} dataset to {Data.max_entries} entries')
        rdf = rdf.Range(Data.max_entries)

    return rdf
#---------------------------------
def _define_columns(rdf):
    '''
    Will define variables
    '''
    d_var = Data.cfg_dict['dataset']['define']

    log.info('Defining variables')
    for name, expr in d_var.items():
        log.debug(f'---> {name}')
        rdf = rdf.Define(name, expr)

    return rdf
#---------------------------------
def _apply_selection(rdf, kind):
    '''
    Will take ROOT dataframe and kind (bkg or sig)
    Will load selection from config
    Will return dataframe after selection
    '''

    log.info('Applying selection')
    d_cut = Data.cfg_dict['dataset']['selection'][kind]
    d_cut['q2bin'] = Data.cfg_dict['q2'][Data.q2bin]

    for name, cut in d_cut.items():
        log.debug(f'---> {name}')
        rdf = rdf.Filter(cut, name)

    log.info(f'Cutflow for: {kind}')
    rep = rdf.Report()
    rep.Print()

    return rdf
#---------------------------------
def main():
    '''
    Script starts here
    '''

    plt.style.use(mplhep.style.LHCb2)

    _get_args()
    _load_config()

    rdf_bkg = _get_rdf(kind='bkg')
    rdf_sig = _get_rdf(kind='sig')

    trn = TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=Data.cfg_dict)
    trn.run()
#---------------------------------
if __name__ == '__main__':
    main()
