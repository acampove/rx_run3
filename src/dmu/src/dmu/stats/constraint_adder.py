'''
This module contains the ConstraintAdder class
'''
import numpy
import zfit
from dmu.logging.log_store import LogStore

from typing          import Union, cast
from omegaconf       import DictConfig
from zfit            import Parameter
from zfit.loss       import ExtendedUnbinnedNLL, UnbinnedNLL
from .constraint     import Constraint1D, ConstraintND

log        = LogStore.add_logger('dmu:stats:constraint_adder')
Constraint = Union[Constraint1D, ConstraintND]
Loss       = Union[ExtendedUnbinnedNLL, UnbinnedNLL]
# ----------------------
class ConstraintAdder:
    '''
    This class is in charge of:

    - Transforming a config object into constrain objects
    - Using those constraints to update the NLL
    '''
    # ----------------------
    def __init__(
        self, 
        nll         : Loss, 
        constraints : list[Constraint]):
        '''
        Parameters
        -------------
        nll        : Zfit likelihood, before constraints added
        constraints: List of constraint objects
        '''
        self._nll         = nll
        self._constraints = constraints

        self._d_par = self._get_params(nll=nll)
        self._d_cns : dict[str,Parameter] = {}
    # ----------------------
    def _get_params(self, nll : Loss) -> dict[str, Parameter]:
        '''
        Parameters
        -------------
        nll: Likelihood holding parameters

        Returns
        -------------
        Dictionary mapping parameter names with parameters
        '''
        s_par = nll.get_params(floating=True)
        if len(s_par) == 0:
            raise ValueError('No floating parameter found in likelihood')

        return { par.name : cast(Parameter, par) for par in s_par }
    # ----------------------
    def _get_observation(self, cfg : DictConfig) -> list[Parameter]:
        '''
        Parameters
        -------------
        cfg  : Configuration specifying how to build the Gaussian constraint
        mode : Controls the observation value. Either toy or real.

        Returns
        -------------
        List of observations as parameters
        '''
        l_nam = cfg.parameters
        l_val = cfg.observation
        l_par = [ zfit.Parameter(f'{nam}_cns', fval) for nam, fval in zip(l_nam, l_val) ]

        return l_par
    # ----------------------
    def _update_observations(self, values : numpy.ndarray, names : list[str]) -> None:
        '''
        This method sets the values of the constraining parameters from resampled values

        Parameters
        -------------
        values: Array with resampled observations
        names : Names of parameters used to constrain likelihood
        '''
        for name, value in zip(names, values):
            log.verbose(f'Setting {name}={value}')

            if name not in self._d_cns:
                raise ValueError(f'Cannot find constraining parameter: {name}')

            par = self._d_cns[name]
            par.set_value(value)
    # ----------------------
    def get_nll(self) -> Loss:
        '''
        Returns
        -------------
        Likelihood with constrain added
        '''
        if self._constraints is None:
            log.info('No constraints found, using original (unconstrained) NLL')
            return self._nll

        l_const = [ 
            cns.zfit_cons(holder = self._nll) # type: ignore
            for cns in self._constraints ]

        nll = self._nll.create_new(constraints=l_const)
        if nll is None:
            raise ValueError('Could not create a new likelihood')

        return nll
    # ----------------------
    def resample(self) -> None:
        '''
        Will update the parameters associated to constraint
        '''
        if self._cns is None:
            log.debug('Not resampling constraints for case without constraints')
            return

        for name, cfg_block in self._cns.items():
            log.verbose(f'Resampling block: {name}')
            self._resample_block(cfg=cfg_block)
# ----------------------
