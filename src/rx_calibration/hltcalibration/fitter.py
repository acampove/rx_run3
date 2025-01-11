'''
Module containing Fitter class
'''
# pylint: disable=import-error, unused-import

import ROOT
import zfit
from zfit.core.interfaces    import ZfitSpace

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
        self._pdf_ful : BasePDF

        self._par_nsg = zfit.Parameter('nsig', 10, 0, 100_000)
        self._par_nbk = zfit.Parameter('nbkg', 10, 0, 100_000)

        self._obs      : ZfitSpace
        self._obs_name : str
    # -------------------------------
    def _initialize(self):
        self._check_extended()
        self._check_sim_weights()

        self._obs      = self._pdf_sig.space
        self._obs_name,= self._pdf_sig.obs

        esig           = self._pdf_sig.create_extended(self._par_nsg)
        ebkg           = self._pdf_bkg.create_extended(self._par_nbk)
        self._pdf_ful  = zfit.pdf.SumPDF([esig, ebkg])

        log.info(f'Using observable: {self._obs_name}')
    # -------------------------------
    def _check_sim_weights(self) -> None:
        v_col = self._rdf_sim.GetColumnNames()
        l_col = [col.c_str() for col in v_col]

        if 'weights' in l_col:
            log.debug('Weights column found, not defining ones')
            return

        log.debug('Weights column not found, defining weights as ones')
        self._rdf_sim = self._rdf_sim.Define('weights', '1')
    # -------------------------------
    def _check_extended(self) -> None:
        if self._pdf_sig.is_extended:
            raise ValueError('Signal PDF should not be extended')

        if self._pdf_bkg.is_extended:
            raise ValueError('Background PDF should not be extended')
    # -------------------------------
    def _fit_signal(self) -> Parameter:
        return Parameter()
    # -------------------------------
    def _fit_data(self) -> Parameter:
        return Parameter()
    # -------------------------------
    def _fix_tails(self, sig_par : Parameter) -> None:
        pass
    # -------------------------------
    def fit(self) -> Parameter:
        '''
        Function returning Parameter object holding fitting parameters
        '''
        self._initialize()
        sig_par = self._fit_signal()
        self._fix_tails(sig_par)

        dat_par = self._fit_data()

        return dat_par
# --------------------------------------------------
