'''
Module with FWHM plugin class
'''
import zfit
import numpy
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:plotting:fwhm')
# --------------------------------------------
class FWHM:
    '''
    Class meant to be used to calculate Full Width at Half Maximum
    as a Plotter1d plugin
    '''
    # -------------------------
    def __init__(self, cfg : dict, val : numpy.ndarray, wgt : numpy.ndarray, maxy : float):
        self._cfg     = cfg
        self._arr_val = val
        self._arr_wgt = wgt
        self._maxy    = maxy
    # -------------------------
    def _normalize_yval(self, arr_pdf_val : numpy.ndarray) -> None:
        max_pdf_val = numpy.max(arr_pdf_val)
        arr_pdf_val*= self._maxy / max_pdf_val

        return arr_pdf_val
    # -------------------------
    def run(self):
        '''
        Runs plugin
        '''
        minx = numpy.min(self._arr_val)
        maxx = numpy.max(self._arr_val)

        log.info('Running FWHM pluggin')
        obs = zfit.Space('mass', limits=(minx, maxx))
        pdf= zfit.pdf.KDE1DimExact(obs=obs, data=self._arr_val, weights=self._arr_wgt)

        xval = numpy.linspace(minx, maxx, 200)
        yval = pdf.pdf(xval)
        yval = self._normalize_yval(yval)

        plt.plot(xval, yval, linestyle='-', linewidth=2, color='red')
# --------------------------------------------
