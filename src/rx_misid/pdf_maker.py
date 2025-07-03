'''
Module holding PIDWeighter class
'''
import pandas as pnd

from zfit.core.interfaces      import ZfitData   as zobs
from zfit.core.interfaces      import ZfitPDF    as zpdf
from dmu.generic               import utilities  as gut
from dmu.logging.log_store     import LogStore
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
            q2bin  : str):
        '''
        Parameters
        ------------------
        sample: e.g. Bu_KplKplKmn_eq_sqDalitz_DPC
        q2bin : e.g. central
        '''
        self._sample = sample
        self._q2bin  = q2bin
    # -----------------------------------------
    def _pdf_from_df(
            self,
            df : pnd.DataFrame,
            obs : zobs) -> zpdf:
        '''
        Parameters
        ---------------
        df : Pandas dataframe with observable and weights
        obs: Observable used for the PDF
        '''

        print(df)
    # -----------------------------------------
    def get_pdf(self, obs : zobs) -> zpdf:
        '''
        Parameters
        ---------------
        obs: Obserbable used in PDF

        Returns
        ---------------
        pdf: Fitted PDF
        '''
        cfg = gut.load_data(package='rx_misid_data', fpath = 'misid.yaml')

        cfg['input']['sample' ] = self._sample
        cfg['input']['q2bin'  ] = self._q2bin
        cfg['input']['project'] = 'nopid'

        obj = MisIDCalculator(cfg=cfg)
        df  = obj.get_misid()
        pdf = self._pdf_from_df(df=df, obs=obs)

        return pdf
# ------------------------------------------------
