'''
Unit test for Mva class
'''
import pytest

from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:test_train_mva')

# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('data_checks:train_mva', 10)
# -------------------------------
def test_train():
    '''
    Test training
    '''
    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')
    cfg     = ut.get_config('ml/tests/train_mva.yaml')

    obj= TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=cfg)
    obj.run()
# -------------------------------
