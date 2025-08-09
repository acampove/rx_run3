'''
Module containing DataFitter class
'''
from omegaconf                 import DictConfig
from dmu.logging.log_store     import LogStore
from dmu.workflow.cache        import Cache
from dmu.stats.fitter          import Fitter
from dmu.stats                 import utilities as sut
from zfit.exception            import ParamNameNotUniqueError
from fitter.base_fitter        import BaseFitter
from zfit.interface            import ZfitLoss            as NLL

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
        d_nll : dict[str,tuple[NLL,DictConfig]],
        cfg   : DictConfig) -> None:
        '''
        Parameters
        -------------
        d_nll:  Dictionary with:
            Key  : Name of region where to fig, e.g. signal, control
            Value:
                - ExtendedBinnedNLL instance
                - Configuration storing the selections used by default and in the fit
        cfg  : Dictionary with configuration used for fit with:
            - Output directory
            - Fit settings
            - Plotting settings
        '''
        self._d_nll = d_nll
        self._cfg   = cfg
        self._d_cns : dict[str,tuple[float,float]]|None = None

        BaseFitter.__init__(self)
        # TODO: Is the likelihood hashable?
        # If so, it should be here
        Cache.__init__(
            self,
            out_path = self._cfg.output_directory,
            cfg      = cfg)
    # ----------------------
    def _get_full_nll(self) -> NLL:
        '''
        Returns
        -------------
        Full likelihood, i.e. sum over all the models
        '''
        l_nll = [ nll for nll, _ in self._d_nll.values() ]
        nll   = sum(l_nll[1:], l_nll[0])

        return nll
    # ----------------------
    @property
    def constraints(self) -> dict[str,tuple[float,float]]:
        '''
        Returns dictionary with constraints where:
        Key  : Name of parameter to constrain
        Value: Tuple with mu and sigma for Gaussian constrain
        '''
        if self._d_cfg is None:
            return {}

        return self._d_cfg
    # ----------------------
    @constraints.setter
    def constraints(self, value : dict[str,tuple[float,float]]):
        '''
        Parameters
        -------------
        value: Dictionary with:
            key  : Name of the parameter to constrain
            value: Tuple with mu and sigma associated to constrain
        '''
        if len(value) == 0:
            raise ValueError('Passed empty dictionary of constraints')

        if self._d_cns is not None:
            raise ValueError('Cannot set constraints, constraints were already set')

        log.info('Using constraints')

        log.debug(80 * '-')
        log.debug(f'{"Parameter":<50}{"Value":<15}{"Error":<15}')
        log.debug(80 * '-')
        for par_name, (val, err) in value.items():
            log.debug(f'{par_name:<50}{val:<15.3f}{err:<15.3f}')
        log.debug(80 * '-')

        self._d_cns = value
    # ----------------------
    def run(self) -> DictConfig:
        '''
        Entry point for fitter

        Returns
        -------------
        OmegaConf DictConfig with fitting parameters
        '''
        nll    = self._get_full_nll()
        l_cfg  = [ cfg for _, cfg in self._d_nll.values() ]
        l_nam  = list(self._d_nll)

        if self._d_cns is not None:
            cns= Fitter.get_gaussian_constraints(obj=nll, cfg=self._d_cns)
            nll= nll.create_new(constraints=cns) # type: ignore

        res, _ = Fitter.minimize(nll=nll, cfg=self._cfg.fit)

        res.hesse(name='minuit_hesse')

        for model, data, cfg, name in zip(nll.model, nll.data, l_cfg, l_nam):
            out_path = f'{self._out_path}/{name}'

            self._save_fit(
                cut_cfg  = cfg.selection,
                plt_cfg  = self._cfg.plots,
                data     = data,
                model    = model,
                res      = res,
                d_cns    = self._d_cns,
                out_path = out_path)

        cres = sut.zres_to_cres(res=res)

        return cres
# ----------------------
