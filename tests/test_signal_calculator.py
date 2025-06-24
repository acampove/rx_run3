'''
This module contains tests for SignalCalculator class
'''
import pytest

from dmu.generic                     import utilities as gut
from rx_classifier.signal_calculator import SignalCalculator

# --------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str):
    '''
    Simplest test
    '''
    cfg = gut.load_data(package='rx_classifier_data', fpath='optimization/scanning.yaml')
    cal = SignalCalculator(cfg=cfg, q2bin=q2bin)
    df  = cal.get_signal()
# --------------------------------
