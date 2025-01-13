'''
Module containing derived classes from ZFit minimizer
'''

import zfit

# ------------------------
class AnealingMinimizer(zfit.minimize.Minuit):
    '''
    Class meant to minimizer zfit likelihoods by using multiple retries,
    each retry is preceeded by the randomization of the fitting parameters
    '''
    # ------------------------
    def __init__(self, ntries : int, pvalue : float):
        self._ntries = ntries
        self._pvalue = pvalue

        super().__init__()
    # ------------------------
    def minimize(self, *args, **kwargs):
        '''
        Will run minimization and return FitResult object
        '''
        res = super().minimize(*args, **kwargs)

        return res
# ------------------------
