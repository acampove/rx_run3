'''
Module holding DataPreprocessor class
'''
from typing import cast

import numpy

from ROOT                   import RDataFrame
from dmu.workflow.cache     import Cache
from dmu.stats.zfit         import zfit
from dmu.stats              import utilities  as sut
from zfit.core.interfaces   import ZfitData   as zdata
from zfit.core.interfaces   import ZfitSpace  as zobs
from rx_data.rdf_getter     import RDFGetter

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
    def _get_toy_array(self, sample : str) -> numpy.ndarray:
        '''
        Returns array with toy data
        '''
        if sample == 'gauss_toy':
            sig  = numpy.random.normal(loc=5280, scale=50, size=10_000)
            return sig

        if sample == 'data_toy':
            sig = self._get_toy_array(sample='gauss_toy')
            bkg = numpy.random.exponential(scale=10_000, size=100_000)
            arr = numpy.concatenate((sig, bkg))

            return arr

        raise NotImplementedError(f'Cannot retrive toy data for sample: {sample}')
    # ------------------------
    def _get_array(self) -> numpy.ndarray:
        if 'toy' in self._sample:
            return self._get_toy_array(sample=self._sample)

        gtr = RDFGetter(sample =self._sample, trigger=self._trigger)
        rdf = gtr.get_rdf()
        rdf = cast(RDataFrame, rdf)

        name = sut.name_from_obs(obs=self._obs)
        arr  = rdf.AsNumpy([name])[name]

        return arr
    # ------------------------
    def get_data(self) -> zdata:
        '''
        Returns zfit data
        '''
        arr  = self._get_array()
        data = zfit.data.from_numpy(obs=self._obs, array=arr)

        return data
# ------------------------
