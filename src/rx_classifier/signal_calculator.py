'''
Module with SignalCalculator class
'''
import pandas as pnd

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
        self._cfg = cfg
        self._q2bin = q2bin
    # -----------------------------------
    def get_signal(self) -> pnd.DataFrame:
        '''
        Reuturns pandas dataframe with signal yields
        '''
        return
# -----------------------------------
