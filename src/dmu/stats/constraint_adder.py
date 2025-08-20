'''
This module contains the ConstraintAdder class
'''
from typing          import Union, cast

import zfit
from omegaconf       import DictConfig, DictKeyType
from zfit            import Parameter
from zfit.constraint import GaussianConstraint, PoissonConstraint
from zfit.loss       import ExtendedUnbinnedNLL, UnbinnedNLL

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:stats:constraint_adder')
Constraint = Union[GaussianConstraint, PoissonConstraint]
Loss       = Union[ExtendedUnbinnedNLL, UnbinnedNLL]
# ----------------------
class ConstraintAdder:
    '''
    This class is in charge of:

    - Transforming a config object into constrain objects
    - Using those constraints to update the NLL
    '''
    # ----------------------
    def __init__(self, nll : zlos, cns : DictConfig):
        '''
        Parameters
        -------------
        nll: Zfit likelihood, before constraints added
        cns: Configuration, describing 
            - What variables to constraint
            - What kind of constraint to use
            - What the means of the contraints should be
            - What the covariances should be
        '''
        self._nll = nll
        self._cns = cns

        self._valid_modes = ['real', 'toy']
    # ----------------------
    def get_nll(self, mode : str) -> zlos:
        '''
        Parameters
        -------------
        mode: Describes what kind of constraint to attach
                real_fit: Pick parameters as they are in the input config
                toys    : Draw parameters from distribution. Meant to be used for toy fitting

        Returns
        -------------
        Likelihood with constrain added
        '''
        if mode not in self._valid_modes:
            raise ValueError(f'Invalide mode {mode} pick among: {self._valid_modes}')

        return self._nll
# ----------------------
