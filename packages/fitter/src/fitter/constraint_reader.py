'''
Script holding ConstraintReader class
'''

from dmu.logging.log_store       import LogStore
from rx_efficiencies.decay_names import DecayNames    as dn
from fitter.prec_scales          import PrecScales
from fitter.protocols            import ParameterHolder

log=LogStore.add_logger('fitter:constraint_reader')
# -------------------------------------------------------------
class ConstraintReader:
    '''
    Class meant to provide constraints for fitting model
    '''
    # -------------------------------------------------------------
    def __init__(
        self, 
        obj   : ParameterHolder, 
        q2bin : str,
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
        self._l_par   = [ par.name for par in s_par ] 
        self._q2bin   = q2bin

        self._d_const = {}
        self._signal  = signal 
        self._prc_pref= pprefx 
    # -------------------------------------------------------------
    def _add_signal_constraints(self) -> None:
        raise NotImplementedError('This needs to be implemented with DataFitter')
    # -------------------------------------------------------------
    def _proc_from_par(self, par_name : str) -> str:
        prefix = f'{self._prc_pref}_yld_'
        if not par_name.startswith(prefix):
            raise ValueError(f'Prec scale parameter does not start with {prefix} but {par_name}')

        sample = par_name.removeprefix(prefix)
        decay  = dn.nic_from_sample(sample)

        return decay
    # -------------------------------------------------------------
    def _add_prec_constraints(self) -> None:
        log.info('Adding partially reconstructed component constraint')
        for par in self._l_par:
            if not par.startswith('pscale'): # PRec constraints are scales, starting with "s"
                continue

            log.debug(f'Adding constrint for: {par}')

            process  = self._proc_from_par(par)
            obj      = PrecScales(proc=process, q2bin=self._q2bin)
            val, err = obj.get_scale(signal=self._signal)

            self._d_const[par] = val, err
    # -------------------------------------------------------------
    def get_constraints(self) -> dict[str,tuple[float,float]]:
        '''
        Returns dictionary with constraints, i.e.

        Key  : Name of fitting parameter
        Value: Tuple with mu and error
        '''
        self._add_prec_constraints()

        return self._d_const
# -------------------------------------------------------------
