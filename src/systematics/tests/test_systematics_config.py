'''
Module holding tests for Systematics configuration class
'''
import pytest

from rpk_tools     import RareEESystematics
from rx_generic    import utilities as gut
from rpk_log_store import log_store as LogStore

log = LogStore.add_logger('rpk_tools:test_systematics')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rpk_tools:fit_config', 10)
# --------------------------------
def test_model_systematics():
    '''
    Test of model systematics
    '''
    data = gut.load_data(
        package = 'rpk_configs', 
        fpath   = 'toys/sys_rare_ee.yaml')
    syst = RareEESystematics(**data)

    cfg_sig, _ = syst.vary_model(kind = 'sig')
    cfg_cmb, _ = syst.vary_model(kind = 'cmb')

    assert cfg_sig.sig.nominal == syst.sig.variation
    assert cfg_cmb.cmb.nominal == syst.cmb.variation
# --------------------------------
def test_kde_systematics():
    '''
    Test of model systematics
    '''
    data = gut.load_data(
        package = 'rpk_configs', 
        fpath   = 'toys/sys_rare_ee.yaml')
    syst = RareEESystematics(**data)

    while syst.npvs:
        log.info(syst.this_npv_name)
        syst.npv
# --------------------------------
