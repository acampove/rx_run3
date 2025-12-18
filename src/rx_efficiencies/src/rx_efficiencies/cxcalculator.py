'''
Module holding CXCalculator
'''
import random

from typing        import Final
from dmu           import LogStore
from rx_common     import Project, Trigger, Sample
from rx_common     import Channel, Qsq
from uncertainties import Variable, ufloat

from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

log=LogStore.add_logger('rx_efficiencies:cxcalculator')
# ----------------------
class CXCalculator:
    '''
    Class meant to calculate double ratio of efficiencies
    '''
    # ----------------------
    def __init__(self, project : Project) -> None:
        '''
        Parameters
        -------------
        project: rk or rkst
        '''
        self._project = project
    # ----------------------
    def calculate(self) -> tuple[float,float]:
        '''
        Returns
        -------------
        Tuple with value of cx and its error
        '''
        return 1, 1
# ----------------------
