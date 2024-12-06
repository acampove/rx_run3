'''
Module with tests for BkkChecker class
'''
from importlib.resources import files

import pytest
import yaml

from ap_utilities.logging.log_store       import LogStore
from ap_utilities.bookkeeping.bkk_checker import BkkChecker

log = LogStore.add_logger('ap_utilities:tests:test_bkk_check')
# ----------------------------------------
def _sections_from_path( path : str) -> dict[str, dict]:
    with open(path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    return d_cfg
# ----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('ap_utilities:Bookkeeping.bkk_checker', 10)
# ----------------------------------------
def test_simple():
    '''
    Will save list of samples to YAML
    '''
    samples_path = files('ap_utilities_data').joinpath('rd_samples.yaml')
    samples_path = str(samples_path)

    d_cfg      = _sections_from_path(samples_path)
    d_sections = d_cfg['sections']
    for name, d_section in d_sections.items():
        log.info(f'Processing section: {name}')
        obj=BkkChecker(name, d_section)
        obj.save()
# ----------------------------------------
def test_nick_evt():
    '''
    Will test reading when there are both evt_type and nickname sections 
    '''
    samples_path = files('ap_utilities_data').joinpath('nick_evt.yaml')
    samples_path = str(samples_path)

    d_cfg        = _sections_from_path(samples_path)
    d_sections   = d_cfg['sections']
    for name, d_section in d_sections.items():
        log.info(f'Processing section: {name}')
        obj=BkkChecker(name, d_section)
        obj.save()
# ----------------------------------------
def test_multithreaded():
    '''
    Will save list of samples to YAML using 4 threads
    '''
    samples_path = files('ap_utilities_data').joinpath('rd_samples.yaml')
    samples_path = str(samples_path)

    d_cfg = _sections_from_path(samples_path)
    d_sections = d_cfg['sections']
    for name, d_section in d_sections.items():
        log.info(f'Processing section: {name}')
        obj=BkkChecker(name, d_section)
        obj.save(nthreads=8)
# ----------------------------------------
