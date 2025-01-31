'''
Module containing EffCal class
'''

from rx_calibration.hltcalibration.parameter import Parameter

# --------------------------------------------------
class EffCal:
    '''
    Class used to calculate efficiencies from a Parameter instance
    '''
    def __init__(self, pas : Parameter, fal : Parameter, sig_name : str = 'nsign'):
        '''
        First two arguments are the parameter objects, where the yields are stored.
        The last argument is the name of the signal yield, by default `nsign`.
        '''

        self._pas = pas
        self._fal = fal

        self._sig_name  = sig_name
    # ----------------------------
    def get_eff(self) -> float:
        '''
        Returns float with value of efficiency
        '''
        pas_val, _ = self._pas[self._sig_name]
        fal_val, _ = self._fal[self._sig_name]
        eff        = pas_val / (pas_val + fal_val)

        return eff
# --------------------------------------------------
