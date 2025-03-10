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
    def __init__(self, cfg : dict, val : numpy.ndarray, wgt : numpy.ndarray):
        self._cfg     = cfg
        self._arr_val = val
        self._arr_wgt = wgt
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
        yval = 1000 * pdf.pdf(xval)

        plt.plot(xval, yval)
# --------------------------------------------
