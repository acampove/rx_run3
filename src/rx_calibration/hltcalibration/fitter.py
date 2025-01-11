'''
Module containing Fitter class
'''
import ROOT
import zfit

from zfit.core.basepdf     import BasePDF
from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore

from rx_calibration.hltcalibration.parameter import Parameter

log = LogStore.add_logger('rx_calibration:fitter')
# --------------------------------------------------
class Fitter:
    '''
    Class meant to produce a Parameter object
    from:

    - Simulated and real data stored in ROOT dataframe
    - Signal and background zfit PDFs
    '''
    def __init__(self, data : RDataFrame, sim : RDataFrame, smod : BasePDF, bmod : BasePDF):
        self._rdf_dat = data
        self._rdf_sim = sim

        self._pdf_sig = smod
        self._pdf_bkg = bmod

        self._obs      : zfit.Space
        self._obs_name : str
    # -------------------------------
    def _initialize(self):
        self._obs      = self._pdf_sig.obs
        self._obs_name = self._obs.name

        log.info(f'Using observable: {self._obs_name}')
    # -------------------------------
    def fit(self) -> Parameter:
        '''
        Function returning Parameter object holding fitting parameters
        '''
        self._initialize()

        par = Parameter()

        return par
# --------------------------------------------------
