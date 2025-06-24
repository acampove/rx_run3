'''
This module contains tests for SignalCalculator class
'''

from dmu.generic                     import utilities as gut
from rx_classifier.signal_calculator import SignalCalculator

# --------------------------------
def test_simple():
    '''
    Simplest test
    '''
    cfg = gut.load_data(package='rx_classifier_data', fpath='optimization/scanning.yaml')
    cal = SignalCalculator(cfg=cfg)
    df  = cal.get_signal()
# --------------------------------
