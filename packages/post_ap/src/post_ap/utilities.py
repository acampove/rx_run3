'''
Module with utility functions
'''

import os

import yaml
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('post_ap:utilities')
# --------------------------------------
def load_config() -> dict:
    '''
    Uses the env variable CONFIG_PATH to find the path to the YAML file.

    Returns
    -----------------
    d_config (dict): Dictionary with configuration
    '''
    cfg_path = os.environ['CONFIG_PATH']

    if not os.path.isfile(cfg_path):
        raise FileNotFoundError(f'Config path not found: {cfg_path}')

    log.debug(f'Loading: {cfg_path}')
    with open(cfg_path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    return data
# --------------------------------------
