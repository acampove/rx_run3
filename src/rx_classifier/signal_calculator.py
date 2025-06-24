'''
Module with SignalCalculator class
'''
import pandas as pnd
from dmu.logging.log_store              import LogStore
from dmu.generic                        import utilities as gut
from rx_efficiencies.efficiency_scanner import EfficiencyScanner
from rx_efficiencies.decay_names        import DecayNames

log=LogStore.add_logger('rx_classifier:signal_calculator')
# -----------------------------------
class SignalCalculator:
    '''
    Class meant to calculate expected value of
    signal yield for different working points
    '''
    # -----------------------------------
    def __init__(self, cfg : dict, q2bin : str):
        '''
        Picks configuration
        '''
        self._cfg   = cfg
        self._q2bin = q2bin
    # -----------------------------------
    def get_signal(self) -> pnd.DataFrame:
        '''
        Reuturns pandas dataframe with signal yields
        '''
        return
# -----------------------------------
