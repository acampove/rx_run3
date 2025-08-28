'''
Module containing the MisIDConstraints class
'''
from dmu.stats.fitter      import GofCalculator
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

from dmu.generic           import utilities        as gut
from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.stats.zfit        import zfit
from dmu.logging.log_store import LogStore
from dmu.workflow.cache    import Cache

from omegaconf      import DictConfig, OmegaConf
from zfit.interface import ZfitSpace           as zobs
from zfit.interface import ZfitPDF             as zpdf
from zfit.loss      import ExtendedUnbinnedNLL as zlos

from fitter.sim_fitter         import SimFitter
from fitter.data_fitter        import DataFitter
from fitter.likelihood_factory import LikelihoodFactory
from fitter.data_preprocessor  import DataPreprocessor

log=LogStore.add_logger('fitter:misid_constraints')
# -------------------------        
class MisIDConstraints(Cache):
    '''
    Class meant to provide constraints for the yields of different
    misID components, e.g.:

    - kpipi:
        - 3
        - 1
    - kkk:
        - 4
        - 2
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
        self._trigger   = 'Hlt2RD_BuToKpEE_MVA_noPID'
        self._project   = 'nopid'

        d_sel = sel.selection(
            q2bin   = self._q2bin, 
            trigger = self._trigger, 
            process = 'DATA_24_*'),

        Cache.__init__(
            self,
            out_path = f'{self._cfg.output_directory}/{q2bin}',
            q2bin    = self._q2bin,
            d_sel    = d_sel, 
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

        nkkk      = zfit.param.ConstantParameter('nkkk'  , npars.nkkk  )
        nkpipi    = zfit.param.ConstantParameter('nkpipi', npars.nkpipi)

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
            trigger  = self._trigger,
            project  = self._project,
            q2bin    = self._q2bin)
        pdf = ftr.get_model()
        if pdf is None:
            raise ValueError(f'Could not retrieve PDF for: {kind}')

        return pdf
    # ----------------------
    def _get_constraints(self, pars : DictConfig) -> dict[str,tuple[float,float]]:
        '''
        Parameters
        -------------
        pars: Dictionary with parameters from fit to control region

        Returns
        -------------
        Dictionary with yields in signal region
        '''
        d_yield   = {}
        for nickname in ['kpipi', 'kkk']:
            d_yield[f'yld_{nickname}'] = self._get_signal_region_yield(nickname = nickname, pars=pars) 

        return d_yield 
    # ----------------------
    def _get_signal_region_yield(self, nickname : str, pars : DictConfig) -> tuple[float,float]:
        '''
        Parameters
        -------------
        nickname : Nickname of fully hadronic misID sample, e.g. kpipi
        pars     : Dictionary with fitting parameters for fit to control region

        Returns
        -------------
        Tuple with expected signal region yield and error
        '''
        control_yield = pars[f'yld_{nickname}_{nickname}'].value
        control_error = pars[f'yld_{nickname}_{nickname}'].error
        scale         = self._get_transfer_factor(nickname=nickname)

        # Use it to scale yield from control region in data
        value = control_yield * scale
        error = control_error * scale

        log.info(f'Control yield: {control_yield:.3f}')
        log.info(f'Signal yield: {value:.3f}')
        log.info(20 * '-')

        return value, error
    # ----------------------
    def _get_transfer_factor(self, nickname : str) -> float:
        '''
        Parameters
        -------------
        nickname: Sample nickname, e.g. kkk, kpipi        

        Returns
        -------------
        Ratio of PID efficiencies between signal and control region
        Needed to translate MisID yields in control region to expectation
        in signal region
        '''
        sample  = self._cfg.model.components[nickname]['sample']
        wgt_cfg = self._cfg.model.components[nickname].categories.main.weights

        sig_yld, ctr_yld = 0, 0 
        pid_sel          = {'pid_l' : '(1)'}

        # Extract yields from weighted (PID) no PID misID MC
        log.info(20 * '-')
        for is_sig in [True, False]:
            prp = DataPreprocessor(
                obs    = self._obs,
                out_dir= nickname,
                sample = sample,
                trigger= 'Hlt2RD_BuToKpEE_MVA_noPID',
                project= 'nopid',
                wgt_cfg= wgt_cfg,
                is_sig = is_sig,
                cut    = pid_sel,
                q2bin  = self._q2bin)
            dat = prp.get_data()
            yld = dat.weights.numpy().sum()
            yld = float(yld)

            log.info(f'IsSig {is_sig} MC yield: {yld:.3f}')

            if is_sig:
                sig_yld = yld
            else:
                ctr_yld = yld

        return sig_yld / ctr_yld
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
        cut = f'({cut_l1}) && ({cut_l2})'

        log.info('')
        log.info(f'Building {kind} PID control region with:')
        log.info(cut)
        log.info('')

        return cut
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
    def get_constraints(self) -> dict[str,tuple[float,float]]:
        '''
        Returns
        -------------
        Constraints on normalization of misID components, i.e.:

        kkk   : (yield, error)
        kpipi : (yield, error)
        '''
        cons_path = f'{self._out_path}/constraints.yaml'
        if self._copy_from_cache():
            log.info(f'Found cached: {cons_path}')

            d_cns = gut.load_json(cons_path)

            return d_cns 

        log.info(f'Running full calculation, nothing cached in: {cons_path}')
        nll_kpp = self._get_control_nll(kind='kpipi')
        nll_kkk = self._get_control_nll(kind='kkk'  )
        d_nll   = {'kpipi' : nll_kpp, 'kkk' : nll_kkk}

        with GofCalculator.disabled(value=True):
            ftr  = DataFitter(name=self._q2bin, d_nll=d_nll, cfg=self._cfg)
            pars = ftr.run(kind='conf')

        d_cns = self._get_constraints(pars=pars)
        gut.dump_json(data=d_cns, path=cons_path)

        self._cache()

        return d_cns 
# -------------------------
