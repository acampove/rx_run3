'''
Module holding KDEOptimizer class
'''

from dmu.stats.zfit         import zfit

from zfit.core.interfaces   import ZfitData   as zdata
from zfit.core.interfaces   import ZfitPDF    as zpdf
from zfit.core.interfaces   import ZfitSpace  as zobs

# -----------------------------------------------------
class KDEOptimizer:
    '''
    Class meant to wrap KDE1DimFFT in order to pick bandwidth
    that optimizes the goodness of fit
    '''
    # --------------------------
    def __init__(self, data : zdata, obs : zobs):
        '''
        data: zfit data
        obs : observable
        '''
        self._data = data
        self._obs  = obs
    # --------------------------
    def get_pdf(self) -> zpdf:
        '''
        Returns KDE1DimFFT with optimal bandwidth
        '''
        pdf = zfit.pdf.KDE1DimFFT(obs=self._obs, data=self._data)

        return pdf
# -----------------------------------------------------
