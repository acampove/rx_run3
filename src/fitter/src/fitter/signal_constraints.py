'''
Module holding SignalConstraints class
'''

from dmu.stats import Constraint
from zfit.loss import ExtendedUnbinnedNLL

# ------------------------------------------
class SignalConstraints:
    '''
    Class used to provide constraints for signal, given the likelihood
    '''
    # ----------------------
    def __init__(
        self, 
        name : str,
        nll  : ExtendedUnbinnedNLL):
        '''
        Parameters
        -------------
        name: Name of signal component, needed to find parameters in NLL
        nll : Likelihood
        '''
        self._name = name 
        self._nll  = nll

    # ----------------------
    def get_constraints(self) -> list[Constraint]:
        '''
        Returns
        -------------
        List of constraints
        '''
        return []
# ------------------------------------------
