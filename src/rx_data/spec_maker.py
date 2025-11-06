'''
Module with SpecMaker class
'''
from rx_common.types import Trigger

class SpecMaker:
    '''
    Class meant to:

    - Find samples and use them to create a JSON file with them
    - Save file and make path available to user
    '''
    # ----------------------
    def __init__(self, sample : str, trigger : Trigger) -> None:
        '''
        Parameters
        -------------
        sample : E.g. Bu_JpsiK_ee_eq_DPC
        trigger: Hlt2RD_BuToKpEE_MVA
        '''
        self._sample = sample
        self._trigger= trigger

    def get_
        
