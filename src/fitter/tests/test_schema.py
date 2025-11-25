'''
This module holds tests to validate the configs used
by the YAML config files
'''

from pathlib import Path

import pytest
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:test_schema')
# ----------------------
def _get_all_confs(name : str) -> list[str]:
    '''
    Parameters
    -------------
    name: Name of config to find in `fitter_data`, e.g `data.yaml`

    Returns
    -------------
    List of paths inside `fitter_data`
    '''
    root_dir = 'src/fitter_data'
    paths    = Path(root_dir).rglob(name)
    l_path   = list(paths)

    if len(l_path) == 0:
        raise ValueError(f'No config {name} found')

    l_rel_path = [ str(path.relative_to(root_dir)) for path in l_path ]

    return l_rel_path
# ----------------------
@pytest.mark.schema
@pytest.mark.parametrize('name', ['data_config.yaml'])
def test_config(name : str) -> None:
    '''
    This test validates the schema of `data.yaml`
    configs
    '''
    log.info('Testing config')

    l_fpath = _get_all_confs(name=name)

    with gut.enforce_schema_validation(value=True):
        for fpath in l_fpath:
            log.info(fpath)
            gut.load_conf(package='fitter_data', fpath=fpath)
# ----------------------
