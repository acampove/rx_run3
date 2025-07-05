'''
Module holding PIDWeighter class
'''
import pandas            as pnd

from dmu.stats.zfit            import zfit
from dmu.stats                 import utilities  as sut
from dmu.generic               import utilities  as gut
from dmu.logging.log_store     import LogStore

from zfit.core.interfaces      import ZfitSpace  as zobs
from zfit.core.interfaces      import ZfitPDF    as zpdf
from zfit.core.interfaces      import ZfitData   as zdata
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:pdf_maker')
# ------------------------------------------------
class PDFMaker:
    '''
    Class meant to:

    - For a given sample and q2bin provide the corresponding fitted PDF
    '''
    # -----------------------------------------
    def __init__(
            self,
            sample : str,
            trigger: str,
            q2bin  : str):
        '''
        Parameters
        ------------------
        sample : e.g. Bu_KplKplKmn_eq_sqDalitz_DPC
        trigger: HLT2 trigger, likely noPID trigger
        q2bin  : e.g. central
        '''
        self._sample = sample
        self._trigger= trigger
        self._q2bin  = q2bin
    # -----------------------------------------
    def _pdf_from_df(
            self,
            df : pnd.DataFrame,
            obs : zobs) -> tuple[zpdf,zdata]:
        '''
        Parameters
        ---------------
        df : Pandas dataframe with observable and weights
        obs: Observable used for the PDF

        Returns
        ---------------
        Tuple with PDF and data that was used to make it
        '''
        obsname  = sut.name_from_obs(obs=obs)
        arr_mass = df[obsname].to_numpy()
        arr_wgt  = df['weight'].to_numpy()

        data     = zfit.Data.from_numpy (obs=obs, array=arr_mass, weights=arr_wgt)
        pdf      = zfit.pdf.KDE1DimFFT(data=data, obs=obs)

        return pdf, data
    # -----------------------------------------
    # TODO: Add resonant mode to noPID samples
    def _get_project_trigger(self) -> tuple[str,str]:
        if self._sample in ['Bu_JpsiK_ee_eq_DPC']:
            log.warning(f'Using rx samples for {self._sample}')
            trigger = self._trigger.replace('_noPID', '')

            return 'rx', trigger

        return 'nopid', self._trigger
    # -----------------------------------------
    def get_pdf(
            self,
            obs    : zobs,
            region : str) -> zpdf:
        '''
        Parameters
        ---------------
        obs    : Obserbable used in PDF
        region : Region for which weights should be taken, signal or control

        Returns
        ---------------
        zfit PDF the zfit data is attached as `dat`
        '''
        cfg = gut.load_data(package='rx_misid_data', fpath = 'misid.yaml')

        project, trigger = self._get_project_trigger()

        cfg['input']['sample' ] = self._sample
        cfg['input']['trigger'] = trigger
        cfg['input']['q2bin'  ] = self._q2bin
        cfg['input']['project'] = project

        obj = MisIDCalculator(cfg=cfg, mode=region)
        df  = obj.get_misid()
        pdf, dat = self._pdf_from_df(df=df, obs=obs)
        pdf.dat  = dat

        return pdf
# ------------------------------------------------
