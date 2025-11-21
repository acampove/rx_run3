'''
Module containing DataFitter class
'''
from typing                    import Literal, overload
from omegaconf                 import DictConfig, OmegaConf
from dmu.logging.log_store     import LogStore
from dmu.workflow.cache        import Cache
from dmu.stats.fitter          import Fitter
from dmu.stats                 import utilities as sut
from zfit.exception            import ParamNameNotUniqueError
from fitter.base_fitter        import BaseFitter
from zfit.result               import FitResult           as zres
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
        name  : str,
        d_nll : dict[str,tuple[NLL,DictConfig]],
        cfg   : DictConfig) -> None:
        '''
        Parameters
        -------------
        name : Identifier for this fit, e.g. q2bin, needed to name outputs
        d_nll: Dictionary with:
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

        BaseFitter.__init__(self)
        # TODO: Is the likelihood hashable?
        # If so, it should be here
        Cache.__init__(
            self,
            out_path = f'{self._cfg.output_directory}/{name}',
            cfg      = cfg)
    # ----------------------
    def _get_full_nll(self) -> NLL:
        '''
        Returns
        -------------
        Full likelihood, i.e. sum over all the models
        '''
        l_nll = [ nll for nll, _ in self._d_nll.values() ]
        try:
            nll   = sum(l_nll[1:], l_nll[0])
        except ParamNameNotUniqueError:
            for nll in l_nll:
                pdf = nll.model[0]
                sut.print_pdf(pdf=pdf)

            raise ParamNameNotUniqueError('Models contain multiple parameters with the same name')

        return nll
    # ----------------------
    def _save_constraints(self, out_dir : str) -> None:
        '''
        Parameters
        -------------
        out_dir: Path to output directory
        '''
        if 'constraints' not in self._cfg:
            log.info('Constraints not found, not saving them')
            return

        cfg_cns = self._cfg.constraints
        out_path= f'{out_dir}/constraints.yaml'
        log.info(f'Saving constraints to: {out_path}')

        OmegaConf.save(config=cfg_cns, f=out_path, resolve=True)
    # ----------------------
    @overload
    def run(self, kind : Literal['zfit']) -> zres:...
    @overload
    def run(self, kind : Literal['conf']) -> DictConfig:...
    def run(self, kind :             str) -> zres|DictConfig:
        '''
        Entry point for fitter

        Parameters
        -------------
        kind: Type of return value, zfit or conf

        Returns
        -------------
        OmegaConf DictConfig with fitting parameters
        '''
        nll    = self._get_full_nll()
        l_cfg  = [ cfg for _, cfg in self._d_nll.values() ]
        l_nam  = list(self._d_nll)

        if nll is None:
            raise ValueError('Likelihood is missing')

        res, _ = Fitter.minimize(nll=nll, cfg=self._cfg.fit)
        res.hesse(name='minuit_hesse')

        for model, data, cfg, name in zip(nll.model, nll.data, l_cfg, l_nam):
            out_path = f'{self._out_path}/{name}'

            log.info(f'Saving fit to: {out_path}')

            self._save_fit(
                cut_cfg  = cfg.selection,
                plt_cfg  = self._cfg.plots,
                data     = data,
                model    = model,
                res      = res,
                out_path = out_path)

            self._save_constraints(out_dir=out_path)

        if kind == 'zfit':
            return res

        cres = sut.zres_to_cres(res=res)

        return cres
# ----------------------
