'''
Module holding DataPreprocessor class
'''
import numpy

from dmu.workflow.cache     import Cache
from dmu.stats.zfit         import zfit
from zfit.core.interfaces   import ZfitData   as zdata
from zfit.core.interfaces   import ZfitSpace  as zobs

# ------------------------
class DataPreprocessor(Cache):
    '''
    Class in charge of providing datasets for fitting by:

    - Loading ROOT files through RDFGetter
    - Applying selection
    - Transforming data into format that zfit can use
    '''
    # ------------------------
    def __init__(
            self,
            obs     : zobs,
            sample  : str,
            trigger : str,
            project : str,
            q2bin   : str):
        '''
        Parameters
        --------------------
        obs    : zfit observable
        sample : e.g. DATA_24_MagUp...
        trigger: e.g. Hlt2RD...
        project: e.g. rx, nopid
        q2bin  : e.g. central
        '''
        self._obs    = obs
        self._sample = sample
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
    # ------------------------
    def get_data(self) -> zdata:
        '''
        Returns zfit data
        '''
        sig  = numpy.random.normal(loc=5280, scale=50, size=10_000)
        bkg  = numpy.random.exponential(scale=10_000, size=100_000)
        arr  = numpy.concatenate((sig, bkg))
        data = zfit.data.from_numpy(obs=self._obs, array=arr)

        return data
# ------------------------
