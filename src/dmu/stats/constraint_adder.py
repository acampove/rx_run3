'''
This module contains the ConstraintAdder class
'''
from typing          import Union, cast

import numpy
import zfit
from omegaconf       import DictConfig, DictKeyType, OmegaConf
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
    _valid_constraints = ['GaussianConstraint', 'PoissonConstraint']
    # ----------------------
    def __init__(self, nll : Loss, cns : DictConfig):
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
        l_val = cfg.observation
        l_par = [ zfit.Parameter(f'par_{ival:03}', fval) for ival, fval in enumerate(l_val) ]

        return l_par
    # ----------------------
    def _resample_block(self, cfg : DictConfig) -> None:
        '''
        Updates observation values for parameters of a given block of constraints
        '''
        mu  = cfg.observation
        if cfg.kind == 'PoissonConstraint':
            arr = numpy.random.poisson(mu, size=len(mu))
            # Cannot use a lambda=0 for a Poisson distribution
            # Use very small lambda, if RNG gives zero
            arr = numpy.where(arr == 0, 1e-2, arr)
            self._update_observations(values=arr, names=cfg.parameters)

        cov = cfg.cov
        if cfg.kind == 'GaussianConstraint':
            arr = numpy.random.multivariate_normal(mu, cov, size=1)
            self._update_observations(values=arr, names=cfg.parameters)

        raise ValueError(f'Toy observation not defined for: {cfg.kind}')
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
            log.debug(f'Setting {name}={value}')

            if name not in self._d_cns:
                raise ValueError(f'Cannot find constraining parameter: {name}')

            par = self._d_cns[name]
            par.set_value(value)
    # ----------------------
    def _get_gaussian_constraint(self, cfg : DictConfig) -> GaussianConstraint:
        '''
        Parameters
        -------------
        cfg  : Configuration specifying how to build the Gaussian constraint
        mode : Controls the observation value. Either toy or real.

        Returns
        -------------
        Zfit gaussian constrain
        '''
        l_name    = cfg.parameters
        l_obs_par = self._get_observation(cfg=cfg)
        l_obs_val = [ par.value for par in l_obs_par ]
        self._d_cns.update(dict(zip(l_name, l_obs_par)))

        log.verbose('Creating Gaussian constraint')
        log.verbose(f'Observation:\n {l_obs_val}')
        log.verbose(f'Covariance :\n {cfg.cov}')

        s_par = { self._d_par[name] for name in cfg.parameters }
        cns   = zfit.constraint.GaussianConstraint(
            params      = s_par, 
            observation = l_obs_par,
            cov         = cfg.cov)

        return cns
    # ----------------------
    def _get_poisson_constraint(self, cfg : DictConfig, mode : str) -> PoissonConstraint:
        '''
        Parameters
        -------------
        cfg  : Configuration needed to build constraint
        mode : Controls the observation value. Either toy or real.

        Returns
        -------------
        Zfit constraint
        '''
        observation = self._get_observation(cfg=cfg, mode=mode)
        log.verbose('Creating Poisson constraint')
        log.verbose(f'Observation:\n{observation}')

        s_par = { self._d_par[name] for name in cfg.parameters }
        cns   = zfit.constraint.PoissonConstraint(
            params      = s_par, 
            observation = observation)

        return cns
    # ----------------------
    def _create_constraint(self, block : DictKeyType, mode : str) -> Constraint:
        '''
        Parameters
        -------------
        block: Name of the constrain block in the configuration passed in initializer
        mode : Controls the observation value. Either toy or real.

        Returns
        -------------
        Zfit constrain object
        '''
        cfg = self._cns[block]
        if cfg.kind == 'GaussianConstraint':
            return self._get_gaussian_constraint(cfg=cfg, mode=mode)

        if cfg.kind == 'PoissonConstraint':
            return self._get_poisson_constraint(cfg=cfg, mode=mode)

        raise ValueError(f'Invalid constraint type: {cfg.kind}')
    # ----------------------
    @classmethod
    def dict_to_cons(
        cls,
        d_cns : dict[str,tuple[float,float]], 
        name  : str,
        kind  : str) -> DictConfig:
        '''
        Parameters
        -------------
        d_cns: Dictionary mapping variable name to tuple with value and error
        name : Name of block to which these constraints belong, e.g. shape
        kind : Type of constraints, e.g. GaussianConstraint, PoissonConstraint

        Returns
        -------------
        Config object
        '''

        if kind not in cls._valid_constraints:
            raise ValueError(f'Invalid kind {kind} choose from: {cls._valid_constraints}')

        data = None
        if kind == 'PoissonConstraint':
            data = {
                'kind'       : kind,
                'parameters' : list(d_cns),
                'observation': [ val[0] for val in d_cns.values() ]
            }

        if kind == 'GaussianConstraint':
            npar = len(d_cns)
            cov  = []
            for ival, val in enumerate(d_cns.values()):
                zeros       = npar   * [0.]
                var         = val[1] ** 2
                zeros[ival] = var

                cov.append(zeros)

            data = {
                'kind'       : kind,
                'parameters' : list(d_cns),
                'observation': [ val[0] for val in d_cns.values() ],
                'cov'        : cov,
            }

        if data is None:
            raise ValueError('Could not create data needed for constraint object')

        return OmegaConf.create({name : data})
    # ----------------------
    def get_nll(self, mode : str) -> Loss:
        '''
        Parameters
        -------------
        mode: Describes what kind of constraint to attach
                real: Pick parameters as they are in the input config
                toy : Draw parameters from distribution. Meant to be used for toy fitting

        Returns
        -------------
        Likelihood with constrain added
        '''
        if mode not in self._valid_modes:
            raise ValueError(f'Invalide mode {mode} pick among: {self._valid_modes}')

        l_const = [ self._create_constraint(block=block, mode=mode) for block in self._cns ]

        nll = self._nll.create_new(constraints=l_const) # type: ignore
        if nll is None:
            raise ValueError('Could not create a new likelihood')

        return nll
# ----------------------
