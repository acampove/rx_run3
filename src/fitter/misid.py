'''
Module containing the MisID class
'''

from dmu.stats.fitter import GofCalculator
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.stats.zfit        import zfit
from dmu.logging.log_store import LogStore
from dmu.workflow.cache    import Cache

from omegaconf      import DictConfig, OmegaConf
from zfit.interface import ZfitSpace as zobs
from zfit.interface import ZfitPDF   as zpdf
from zfit.interface import ZfitLoss  as zlos 

from fitter.sim_fitter         import SimFitter
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory

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
        obs       : zobs,
        cfg       : DictConfig, 
        q2bin     : str):
        '''
        Parameters
        -------------
        obs      : zfit observable
        cfg      : configuration needed to build PDF
        q2bin    : E.g. central
        '''
        self._obs       = obs
        self._cfg       = cfg
        self._q2bin     = q2bin

        Cache.__init__(
            self,
            out_path = self._cfg.output_directory,
            config   = OmegaConf.to_container(cfg, resolve=True)
        )
    # ----------------------
    def _model_from_pars(self, npars : DictConfig) -> zpdf:
        '''
        Parameters
        -------------
        npars: Dictionary where:
              key  : Name of normalization parameter, e.g. nkpipi, nkkk
              value: Value of parameter

        Returns
        -------------
        Zfit PDF representing misID PDF
        '''
        pdf_kkk   = self._get_misid_pdf(kind='kkk'  ) 
        pdf_kpipi = self._get_misid_pdf(kind='kpipi') 

        nkkk      = zfit.param.ConstantParameter('nkkk'  , npars.kkk  )
        nkpipi    = zfit.param.ConstantParameter('nkpipi', npars.kpipi)

        pdf_kkk.set_yield(nkkk)
        pdf_kpipi.set_yield(nkpipi)

        return zfit.pdf.SumPDF([pdf_kkk, pdf_kpipi])
    # ----------------------
    def _get_misid_pdf(self, kind : str) -> zpdf:
        '''
        Parameters
        -------------
        kind: kind of PDF, e.g. kkk

        Returns
        -------------
        PDF
        '''
        cfg = self._cfg.model.components[kind]
        ftr = SimFitter(
            component= kind,
            cfg      = cfg,
            obs      = self._obs,
            trigger  = 'Hlt2RD_BuToKpEE_MVA_noPID',
            project  = 'nopid',
            q2bin    = self._q2bin)
        pdf = ftr.get_model()
        if pdf is None:
            raise ValueError(f'Could not retrieve PDF for: {kind}')

        return pdf
    # ----------------------
    def _normalization_from_control_region(self, pars : DictConfig) -> DictConfig:
        '''
        Parameters
        -------------
        pars: Dictionary with parameters from fit to control region

        Returns
        -------------
        Dictionary with yields in signal region
        '''
        data = {'nkkk' : 1, 'nkpipi' : 1}
        pars = OmegaConf.create(data)

        return pars
    # ----------------------
    def _get_pid_cut(self, cfg : DictConfig, kind : str) -> str:
        '''
        Parameters
        -------------
        cfg : Config taken from YAML file for data, e.g. data.yaml
        kind: Type of control region, e.g. kkk or kpipi

        Returns
        -------------
        PID cut needed to build control region
        '''
        cut = cfg.selection[kind]

        cut_l1 = cut.replace('LEP_', 'L1_')
        cut_l2 = cut.replace('LEP_', 'L2_')

        # This is the FF region
        return f'({cut_l1}) && ({cut_l2})'
    # ----------------------
    def _get_control_nll(self, kind : str) -> tuple[zlos,DictConfig]:
        '''
        Parameters
        -------------
        kind: Control region type, e.g. kkk, kpipi

        Returns
        -------------
        Tuple with:
            - Likelihood build for requested control region
            - Configuration used to build that likelihood
        '''

        obs     = zfit.Space(f'B_Mass_{kind}', limits=(4500, 7000))
        pid_cut = self._get_pid_cut(cfg=self._cfg, kind=kind)

        with PL.parameter_schema(cfg=self._cfg.model.yields),\
             RDFGetter.default_excluded(names=[]),\
             sel.update_selection(d_sel={'pid_l' : pid_cut}):

            ftr = LikelihoodFactory(
                obs    = obs,
                name   = kind,
                sample = 'DATA_24_*',
                trigger= 'Hlt2RD_BuToKpEE_MVA_ext',
                project= 'rx',
                q2bin  = self._q2bin,
                cfg    = self._cfg)
            nll = ftr.run()
            cfg = ftr.get_config()

        return nll, cfg
    # ----------------------
    def _get_constraints(self) -> dict[str,tuple[float,float]]:
        '''
        This method should provide constraints for ratio of PID efficiencies

        Returns
        -------------
        Dictionary where:
            key  : Name of fitting parameter
            value: Tuple with mu and sigma of constraint
        '''
        return {}
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

            npars = self._normalization_from_control_region(pars=pars)
            model = self._model_from_pars(npars=npars)
            return model

        nll_kpp = self._get_control_nll(kind='kpipi')
        nll_kkk = self._get_control_nll(kind='kkk'  )
        d_nll   = {'kpp_region' : nll_kpp, 'kkk_region' : nll_kkk}

        with GofCalculator.disabled(value=False):
            ftr     = DataFitter(d_nll=d_nll, cfg=self._cfg)
            #ftr.constraints = self._get_constraints()
            pars    = ftr.run()

        OmegaConf.save(pars, pars_path)
        npars   = self._normalization_from_control_region(pars=pars)
        model   = self._model_from_pars(npars=npars)

        self._cache()

        return model
# -------------------------        
