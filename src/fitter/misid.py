'''
Module containing the MisID class
'''

from omegaconf      import DictConfig, OmegaConf
from zfit.interface import ZfitSpace as zobs
from zfit.interface import ZfitPDF   as zpdf
from zfit.loss      import ExtendedUnbinnedNLL

from dmu.logging.log_store import LogStore
from dmu.workflow.cache    import Cache

from fitter.data_fitter import DataFitter

log=LogStore.add_logger('fitter:misid')
# -------------------------        
class MisID(Cache):
    '''
    Class meant to provide PDFs for:

    - Kpip
    - KKK
    '''
    # ----------------------
    def __init__(
        self, 
        component : str, 
        obs       : zobs,
        cfg       : DictConfig, 
        q2bin     : str):
        '''
        Parameters
        -------------
        component: MisID component, e.g. kkk
        obs      : zfit observable
        cfg      : configuration needed to build PDF
        q2bin    : E.g. central
        '''
        self._component = component
        self._obs       = obs
        self._cfg       = cfg
        self._q2bin     = q2bin

        Cache.__init__(
            self,
            out_path = f'{self._cfg.out_path}/{component}',
            config   = cfg,
        )
    # ----------------------
    def get_pdf(self) -> zpdf:
        '''
        Returns
        -------------
        zfit PDF with misid component
        '''
        pars_path = f'{self._out_path}/parameters.yaml'
        if self._copy_from_cache():
            pars = OmegaConf.load(pars_path)
            if not isinstance(pars, DictConfig):
                raise ValueError(f'Parameters are not a dictionary: {pars_path}')

            npar = self._normalization_from_control_region(pars=pars)
            model= self._model_from_pars(npar=npar)

        nll_kpp = self._get_control_nll(kind='kpipi')
        nll_kkk = self._get_control_nll(kind='kkk')
        d_nll   = {'kpp_region' : nll_kpp, 'kkk_region' : nll_kkk}
        ftr     = DataFitter(d_nll=d_nll, cfg=self._cfg.control_fit)
        pars    = ftr.run()
        OmegaConf.save(pars, pars_path)
        npar    = self._normalization_from_control_region(pars=pars)
        model   = self._model_from_pars(npar=npar)

        self._cache()

        return model
# -------------------------        
