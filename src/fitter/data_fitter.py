'''
Module containing DataFitter class
'''
from omegaconf                 import DictConfig
from dmu.logging.log_store     import LogStore
from dmu.workflow.cache        import Cache
from dmu.stats.fitter          import Fitter
from dmu.stats                 import utilities as sut
from rx_selection              import selection as sel
from fitter.base_fitter        import BaseFitter
from fitter.constraint_reader  import ConstraintReader
from zfit.loss                 import ExtendedUnbinnedNLL as NLL

log=LogStore.add_logger('fitter:data_fitter')
# ----------------------
class DataFitter(BaseFitter, Cache):
    '''
    Class meant to take likelihoods for different regions
    and fitting configuration, e.g. constraints and:

    - Build single likelihood with proper constraints
    - Minimize it
    - Save results including plots
    '''
    # ----------------------
    def __init__(
        self,
        d_nll : dict[str,NLL],
        cfg   : DictConfig) -> None:
        '''
        Parameters
        -------------
        d_nll:  Dictionary with:
            Key  : Name of region where to fig, e.g. signal, control
            Value: ExtendedBinnedNLL instance
        '''
        self._d_nll = d_nll
        self._cfg   = cfg

        BaseFitter.__init__(self)
        # TODO: Is the likelihood hashable?
        # If so, it should be here
        Cache.__init__(
            self,
            out_path = 'DataFitter',
            cfg      = cfg)
    # ----------------------
    def _constraints_from_likelihoood(self, nll : NLL) -> dict[str,tuple[float,float]]:
        '''
        Parameters
        ----------------
        nll: Negative log likelihood

        Returns
        ----------------
        Dictionary with:

        Key: Name of parameter
        Value: Tuple value, error. Needed to apply constraints
        '''
        log.info('Getting constraints')

        s_par   = nll.get_params()
        l_par   = [ par.name for par in s_par ]
        obj     = ConstraintReader(parameters = l_par, q2bin = self._q2bin)
        d_cns   = obj.get_constraints()

        log.debug(90 * '-')
        log.debug(f'{"Name":<20}{"Value":<20}{"Error":<20}')
        log.debug(90 * '-')
        for name, (val, err) in d_cns.items():
            log.debug(f'{name:<50}{val:<20.3f}{err:<20.3f}')
        log.debug(90 * '-')

        return d_cns
    # ----------------------
    def run(self) -> DictConfig:
        '''
        Entry point for fitter

        Returns
        -------------
        OmegaConf DictConfig with fitting parameters
        '''
        l_nll  = list(self._d_nll.values())
        nll    = sum(l_nll[1:], l_nll[0])
        cns    = self._constraints_from_likelihoood(nll=nll)
        cns    = Fitter.get_gaussian_constraints(obj=nll, cfg=cns)
        nll    = nll.create_new(constraints=cns)

        res, _ = Fitter.minimize(nll=nll, cfg=self._cfg.fit)

        res.hesse(name='minuit_hesse')
# ----------------------
