'''
Script holding ConstraintReader class
'''

from dmu           import LogStore
from rx_common     import Qsq
from rx_common     import Sample
from .prec_scales  import PrecScales
from dmu.stats     import Constraint, Constraint1D, ParsHolder

log=LogStore.add_logger('fitter:constraint_reader')
# -------------------------------------------------------------
class ConstraintReader:
    '''
    Class meant to provide constraints for fitting model
    '''
    # -------------------------------------------------------------
    def __init__(
        self, 
        obj   : ParsHolder, 
        q2bin : Qsq,
        signal: str = 'bpkpee',
        pprefx: str = 'pscale'):
        '''
        Parameters
        -------------------
        obj   : Object for which `get_parms` is defined, e.g. PDF, likelihood, etc
        q2bin : q2 bin
        signal: Process considered as the signal when calculating denominator of rare Partially
                reconstructed scale constraints
        pprefx: Preffix of rare partially reconstructed parameters, to tell the code
                to read their constraints, if found
        '''
        s_par         = obj.get_params(floating=True) 
        if not s_par:
            raise ValueError('No floating parameters found in parameters holder')

        self._l_par   = [ par.name for par in s_par ] 
        self._q2bin   = q2bin
        self._signal  = signal 
        self._prc_pref= pprefx 

        self._constraints : list[Constraint] = []
    # -------------------------------------------------------------
    def _add_signal_constraints(self) -> None:
        raise NotImplementedError('This needs to be implemented with DataFitter')
    # -------------------------------------------------------------
    def _proc_from_par(self, par_name : str) -> str:
        '''
        Parameters
        ------------------
        par_name : Name of part reco scale parameter to constrain, e.g. pscale_yld_Bp_KpEE_DT...

        Returns
        ------------------
        MC sample nickname for part reco sample, e.g. bdkstkpee
        '''
        prefix = f'{self._prc_pref}_yld_'
        if not par_name.startswith(prefix):
            raise ValueError(f'Prec scale parameter does not start with {prefix} but {par_name}')

        name   = par_name.removeprefix(prefix)
        sample = Sample(name)

        return sample.name 
    # -------------------------------------------------------------
    def _add_prec_constraints(self) -> None:
        '''
        Appends constraints to _constraints list for parameters in parameter holder
        whose names start with `pscale`
        '''
        log.info('Adding partially reconstructed component constraint')
        for par in self._l_par:
            if not par.startswith('pscale'): # PRec constraints are scales, starting with "s"
                continue

            log.debug(f'Adding constrint for: {par}')

            process  = self._proc_from_par(par_name = par)
            obj      = PrecScales(proc=process, q2bin=self._q2bin)
            val, err = obj.get_scale(signal=self._signal)

            cns = Constraint1D(
                name = par,
                kind = 'GaussianConstraint',
                mu   = val,
                sg   = err)

            self._constraints.append(cns)
    # -------------------------------------------------------------
    def get_constraints(self) -> list[Constraint]:
        '''
        Returns dictionary with constraints, i.e.

        Key  : Name of fitting parameter
        Value: Tuple with mu and error
        '''
        self._add_prec_constraints()

        return self._constraints
# -------------------------------------------------------------
