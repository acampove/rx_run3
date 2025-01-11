'''
Module containing EffCal class
'''

from rx_calibration.hltcalibration.parameter import Parameter

# --------------------------------------------------
class EffCal:
    '''
    Class used to calculate efficiencies from a Parameter instance
    '''
    def __init__(self, pas : Parameter, fal : Parameter):
        self._pas = pas
        self._fal = fal
    # ----------------------------
    def get_eff(self) -> float:
        '''
        Returns float with value of efficiency
        '''
        return 0
# --------------------------------------------------
