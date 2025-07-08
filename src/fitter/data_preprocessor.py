'''
Module holding DataPreprocessor class
'''
from typing import cast

import numpy

from ROOT                   import RDataFrame
from dmu.workflow.cache     import Cache
from dmu.stats.zfit         import zfit
from dmu.stats              import utilities  as sut
from dmu.logging.log_store  import LogStore
from zfit.core.interfaces   import ZfitData   as zdata
from zfit.core.interfaces   import ZfitSpace  as zobs
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection  as sel

log=LogStore.add_logger('fitter:data_preprocessor')
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
        self._rdf    = self._get_rdf()

        super().__init__(
                out_path = f'{sample}_{trigger}_{project}_{q2bin}',
                rdf_uid  = self._rdf.uid)
    # ------------------------
    def _get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe after selection
        and with Unique identifier attached as uid
        '''
        log.debug('Retrieving dataframe')
        gtr = RDFGetter(sample =self._sample, trigger=self._trigger)
        rdf = gtr.get_rdf()
        uid = gtr.get_uid()
        rdf = cast(RDataFrame, rdf)

        log.debug('Applying selection')
        rdf = sel.apply_full_selection(
            rdf     = rdf,
            uid     = uid,
            q2bin   = self._q2bin,
            trigger = self._trigger,
            process = self._sample)

        return rdf
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
    def _get_array(self) -> tuple[numpy.ndarray,numpy.ndarray]:
        '''
        Return a tuple of numpy arrays with the observable and weight
        for the sample requested, this array is fully selected
        '''
        if 'toy' in self._sample:
            arr = self._get_toy_array(sample=self._sample)
            wgt = numpy.ones_like(arr)

            return arr, wgt

        rdf = self._rdf
        if log.getEffectiveLevel() < 20:
            rep = rdf.Report()
            rep.Print()

        name = sut.name_from_obs(obs=self._obs)

        log.debug('Retrieving data')
        arr  = rdf.AsNumpy([name])[name]
        wgt  = rdf.AsNumpy(['weight'])['weight']

        nevt = len(arr)
        log.debug(f'Found {nevt} entries')

        return arr, wgt
    # ------------------------
    def get_data(self) -> zdata:
        '''
        Returns zfit data
        '''
        data_path = f'{self._out_path}/data.npz'
        if self._copy_from_cache():
            log.warning(f'Data found cached, loading: {data_path}')
            with numpy.load(data_path) as ifile:
                arr = ifile['values' ]
                wgt = ifile['weights']

            data    = zfit.data.from_numpy(obs=self._obs, array=arr, weights=wgt)
            return data

        arr, wgt = self._get_array()
        data     = zfit.data.from_numpy(obs=self._obs, array=arr, weights=wgt)

        numpy.savez_compressed(data_path, values=arr, weights=wgt)
        self._cache()

        return data
# ------------------------
