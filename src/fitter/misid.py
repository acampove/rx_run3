'''
Module containing the MisID class
'''

from omegaconf      import DictConfig
from zfit.interface import ZfitSpace as zobs
from zfit.interface import ZfitPDF   as zpdf

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:misid')
# -------------------------        
class MisID:
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
    # ----------------------
    def get_pdf(self) -> zpdf:
        '''
        Returns
        -------------
        zfit PDF with misid component
        '''
        return
# -------------------------        
