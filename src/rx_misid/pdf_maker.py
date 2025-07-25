'''
Module holding PIDWeighter class
'''
from typing import cast
import pandas            as pnd

from dmu.stats.zfit            import zfit
from dmu.stats                 import utilities  as sut
from dmu.generic               import utilities  as gut
from dmu.logging.log_store     import LogStore

from zfit.util.ztyping         import XTypeInput as zdata
from zfit.core.interfaces      import ZfitSpace  as zobs
from zfit.core.interfaces      import ZfitPDF    as zpdf
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

        # Will not make PDF with fewer than these entries
        self._min_entries = 50
    # -----------------------------------------
    def _pdf_from_df(
        self,
        df  : pnd.DataFrame,
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

        data     = zfit.Data.from_numpy(obs=obs, array=arr_mass, weights=arr_wgt)
        pdf      = zfit.pdf.KDE1DimFFT(
            name   = self._sample,
            data   = data,
            obs    = obs,
            padding= {'lowermirror' : 0.2, 'uppermirror' : 0.2})

        return pdf, data
    # -----------------------------------------
    def get_pdf(
            self,
            obs    : zobs,
            is_sig : bool) -> zpdf|None:
        '''
        Parameters
        ---------------
        obs    : Obserbable used in PDF
        is_sig : If true, will return signal region PDF, otherwise control region

        Returns
        ---------------
        zfit PDF the zfit data is attached as `dat`
        Or None if fewer than _min_entries were found
        '''
        cfg = gut.load_data(package='rx_misid_data', fpath = 'misid.yaml')

        cfg['input']['sample' ] = self._sample
        cfg['input']['trigger'] = self._trigger
        cfg['input']['q2bin'  ] = self._q2bin
        cfg['input']['project'] = 'nopid'

        obj = MisIDCalculator(cfg=cfg, is_sig=is_sig)
        df  = obj.get_misid()

        if len(df) < self._min_entries: # if there are fewer than this entries, ignore component
            log.warning(f'No candidates found for: {self._sample}/{self._q2bin}/{self._trigger}')
            return None

        pdf, dat = self._pdf_from_df(df=df, obs=obs)
        pdf.dat  = dat

        return pdf
# ------------------------------------------------
