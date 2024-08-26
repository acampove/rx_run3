import toml
import os

from importlib.resources import files
from log_store           import log_store

log=log_store.add_logger('data_checks:utilities')
#--------------------------------------
def load_config(cfg_nam):
    '''
    Parameters
    -----------------
    cfg_nam (str): Name of config file, without extension

    Returns
    -----------------
    d_config (dict): Dictionary with configuration
    '''
    cfg_path = files('data_checks_data').joinpath(f'{cfg_nam}.toml')
    if not os.path.isfile(cfg_path):
        log.error(f'Cannot find config: {cfg_path}')
        raise FileNotFoundError

    log.debug(f'Loading: {cfg_path}')

    return toml.load(cfg_path)
#--------------------------------------

