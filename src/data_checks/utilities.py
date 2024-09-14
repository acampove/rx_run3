import os
import toml
import utils_noroot as utnr

from XRootD              import client
from functools           import cache
from importlib.resources import files
from log_store           import log_store

log = log_store.add_logger('data_checks:utilities')
local_config = False

@cache
# --------------------------------------
def load_config(cfg_nam):
    '''
    Parameters
    -----------------
    cfg_nam (str): Name of config file, without extension

    Returns
    -----------------
    d_config (dict): Dictionary with configuration
    '''
    if not local_config:
        val = _load_grid_config(cfg_nam)
    else:
        val = _load_local_config(cfg_nam)

    return val
# --------------------------------------
def _load_local_config(cfg_nam):
    '''
    Will pick up config file from installed project
    '''
    cfg_path = files('data_checks_data').joinpath(f'{cfg_nam}.toml')
    if not os.path.isfile(cfg_path):
        log.error(f'Config path not found: {cfg_path}')
        raise FileNotFoundError

    log.warning('Loading local config file')

    return toml.load(cfg_path)
# --------------------------------------
@utnr.timeit
def _load_grid_config(cfg_nam):
    '''
    Will use XROOTD to pick up file from grid
    '''
    xrd_path = f'root://x509up_u1000@eoslhcb.cern.ch//eos/lhcb/grid/user/lhcb/user/a/acampove/run3/ntupling/config/{cfg_nam}.toml'
    log.debug(f'Loading: {xrd_path}')
    with client.File() as ifile:
        status, into = ifile.open(xrd_path)
        if not status.ok:
            log.error(status.message)
            raise

        status, file_content = ifile.read()
        if not status.ok:
            log.error(status.message)
            raise

        toml_str  = file_content.decode('utf-8')
        return toml.loads(toml_str)
# --------------------------------------
