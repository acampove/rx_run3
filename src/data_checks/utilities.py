import toml
import os
import utils_noroot as utnr

from XRootD              import client
from functools           import cache
from importlib.resources import files
from log_store           import log_store

log=log_store.add_logger('data_checks:utilities')

@utnr.timeit
@cache
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
#--------------------------------------

