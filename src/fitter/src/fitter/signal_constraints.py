'''
Module holding SignalConstraints class
'''
import re

from typing        import Final
from dmu           import LogStore
from dmu.stats     import Constraint, Constraint1D
from rx_common     import Block, Brem, Correction
from zfit.loss     import ExtendedUnbinnedNLL
from zfit.param    import Parameter as zpar

from .scale_reader import ScaleReader

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
        self._srd   = ScaleReader()

        self._regex           : Final[str] = r'.*signal_brem_00(\d)_b(\d)_\d+_(scale|reso)_flt'
        self._constraint_kind : Final[str] = 'GaussianConstraint'
    # ----------------------
    def _get_constraint(
        self, 
        brem  : Brem, 
        block : Block, 
        kind  : Correction, 
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
        val, err = self._srd.get_scale(corr = kind, block = block, brem = brem)

        log.info(f'Constraining: {par.name}')
        log.debug(f'{"Value":<20}{val:20.3f}')
        log.debug(f'{"Error":<20}{err:20.3f}')

        return Constraint1D(
            name = par.name,
            kind = self._constraint_kind,
            mu   = val,
            sg   = err)
    # ----------------------
    def _get_correction_settings(self, par : zpar) -> tuple[Brem, Block, Correction] | None:
        '''
        Parameters
        -------------
        par: Zfit parameter

        Returns
        -------------
        Tuple with settings to find correction
        or None, if no correction (through constraint) is possible for parameter
        '''
        name = par.name

        mtch = re.match(self._regex, name)
        if not mtch:
            return

        brem = Brem(mtch.group(1)) 
        block= Block(mtch.group(2)) 
        kind = mtch.group(3)

        if   kind == 'scale' and name.startswith('mu_'):
            corr = Correction.mass_scale
        elif kind == 'reso'  and name.startswith('sg_'):
            corr = Correction.mass_resolution
        else:
            raise ValueError(f'Cannot determine correction for parameter: {name}')

        return brem, block, corr
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

            correction_settings = self._get_correction_settings(par = par)
            if correction_settings is None:
                log.debug(f'Not found constraints for {par.name}')
                continue

            brem, block, kind = correction_settings

            constraint = self._get_constraint(
                brem = brem, 
                block= block,
                kind = kind,
                par  = par)

            constraints.append(constraint)

        return constraints
# ------------------------------------------
