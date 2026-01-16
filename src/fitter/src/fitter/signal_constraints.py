'''
Module holding SignalConstraints class
'''
import re

from typing     import Final
from dmu        import LogStore
from dmu.stats  import Constraint, Constraint1D
from zfit.loss  import ExtendedUnbinnedNLL
from zfit.param import Parameter as zpar

log=LogStore.add_logger('fitter:signal_constraints')
# ------------------------------------------
class SignalConstraints:
    '''
    Class used to provide constraints for signal, given the likelihood, it will:

    - Find the parameters to constrain (scale, resolution, etc)
    - Search for the constrain values in a database
    - Build the constrain objects and return them in a list

    Conventions:

    - The signal parameters will be named with `signal_brem_00X_bY`
      as part of the model name, to indicate the brem category and the block.
    '''
    # ----------------------
    def __init__(self, nll  : ExtendedUnbinnedNLL):
        '''
        Parameters
        -------------
        nll : Likelihood
        '''
        self._nll   = nll
        #self._srd   = ScaleReader()

        self._regex           : Final[str] = r'.*signal_brem_00(\d)_b(\d)_\d+_(scale|reso)_flt'
        self._constraint_kind : Final[str] = 'GaussianConstraint'
    # ----------------------
    def _get_constraint(
        self, 
        brem  : str, 
        block : str, 
        kind  : str, 
        par   : zpar) -> Constraint:
        '''
        Parameters
        -------------
        brem : E.g. 1
        block: E.g. 2
        kind : E.g. reso, scale
        par  : Parameter to constrain

        Returns
        -------------
        Constraint object
        '''
        #val, err = self._srd.get_scale(name = kind, block = block, brem = brem)
        val = 3
        err = 1

        log.info(f'Constraining: {par.name}')
        log.debug(f'{"mu":<20}{val:20.3f}')
        log.debug(f'{"sg":<20}{err:20.3f}')

        return Constraint1D(
            name = par.name,
            kind = self._constraint_kind,
            mu   = val,
            sg   = err)
    # ----------------------
    def get_constraints(self) -> list[Constraint]:
        '''
        Returns
        -------------
        List of constraints
        '''
        constraints : list[Constraint] = []

        s_all_par = self._nll.get_params(floating = True, is_yield = False)
        for par in s_all_par:
            if not isinstance(par, zpar):
                raise ValueError(f'Found non-parameter object: {par}')

            mtch = re.match(self._regex, par.name)

            if not mtch:
                continue

            brem = str(mtch.groups(0))
            block= str(mtch.groups(1))
            kind = str(mtch.groups(2))

            constraint = self._get_constraint(
                brem = brem, 
                block= block,
                kind = kind,
                par  = par)

            constraints.append(constraint)

        return constraints
# ------------------------------------------
