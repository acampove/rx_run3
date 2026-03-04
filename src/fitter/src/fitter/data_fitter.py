'''
Module containing DataFitter class
'''
from pathlib         import Path
from typing          import Literal, overload
from dmu             import LogStore
from dmu.workflow    import Cache
from dmu.stats       import FitResult
from dmu.stats       import zfit
from dmu.stats       import utilities as sut
from rx_common       import Qsq
from zfit.exception  import ParamNameNotUniqueError
from zfit.result     import FitResult           as zres
from zfit.loss       import ExtendedUnbinnedNLL as NLL
from .base_fitter    import BaseFitter
from .configs        import FitModelConf

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
        q2bin : Qsq,
        d_nll : dict[str,tuple[NLL,dict[str,dict[str,str]]]],
        cfg   : FitModelConf) -> None:
        '''
        Parameters
        -------------
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
        self._q2bin   = q2bin
        self._d_nll   = d_nll
        self._cfg     = cfg
        self._trigger = cfg.trigger

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
        try:
            nll   = sum(l_nll[1:], l_nll[0])
        except ParamNameNotUniqueError as exc:
            for nll in l_nll:
                pdf = nll.model[0]
                sut.print_pdf(pdf=pdf)

            raise ParamNameNotUniqueError('Models contain multiple parameters with the same name') from exc

        return nll
    # ----------------------
    def _save_constraints(self, out_dir : Path) -> None:
        '''
        Parameters
        -------------
        out_dir: Path to output directory
        '''
        if self._cfg.constraints.is_empty:
            log.info('Constraints not found, not saving them')
            return

        out_path= out_dir / 'constraints.yaml'
        log.info(f'Saving constraints to: {out_path}')

        self._cfg.to_yaml(path = out_path)
    # ----------------------
    @overload
    def run(self, kind : Literal['fres']) -> FitResult:...
    @overload
    def run(self, kind : Literal['zfit']) -> zres:...
    # ----------------------
    def run(self, kind : Literal['zfit', 'fres']) -> zres | FitResult:
        '''
        Entry point for fitter

        Parameters
        -------------
        kind: Type of return value, zfit or conf

        Returns
        -------------
        Either:

        - OmegaConf DictConfig with fitting parameters
        - Zfit result object after freezing
        '''
        nll    = self._get_full_nll()
        l_cfg  = [ cfg for _, cfg in self._d_nll.values() ]
        l_nam  = list(self._d_nll)

        if nll is None:
            raise ValueError('Likelihood is missing')

        min = zfit.minimize.Minuit()
        res = min.minimize(loss = nll)
        res.hesse(name='minuit_hesse', method = 'minuit_hesse')
        fres = FitResult.from_zfit(res = res)

        for model, data, cfg, category in zip(nll.model, nll.data, l_cfg, l_nam, strict = True):
            out_path = self._cfg.output_directory / f'{category}/{self._trigger}_{self._q2bin}'

            log.info(f'Saving fit to: {out_path}')

            self._save_fit(
                cut_cfg  = cfg,
                plt_cfg  = self._cfg.plots,
                data     = data,
                model    = model,
                res      = fres,
                out_path = out_path)

            self._save_constraints(out_dir=out_path)

        if kind == 'zfit':
            return res

        if kind == 'fres':
            return fres 
# ----------------------
