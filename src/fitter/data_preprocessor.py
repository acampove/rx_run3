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
            out_dir : str,
            obs     : zobs,
            sample  : str,
            trigger : str,
            project : str,
            q2bin   : str,
            cut     : str|None = None):
        '''
        Parameters
        --------------------
        out_dir: Directory where caching will happen, with respect to the _cache_root directory
        obs    : zfit observable
        sample : e.g. DATA_24_MagUp...
        trigger: e.g. Hlt2RD...
        project: e.g. rx, nopid
        q2bin  : e.g. central
        cut    : selection that can be added on top. Needed when fits are required in categories, optional
        '''
        self._obs    = obs
        self._sample = sample
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._rdf    = self._get_rdf(cut=cut)
        self._rdf_uid= None if self._rdf is None else self._rdf.uid

        super().__init__(
            out_path = out_dir,
            obs_name = sut.name_from_obs(obs),
            rdf_uid  = self._rdf_uid)
    # ------------------------
    def _get_rdf(self, cut : str|None) -> RDataFrame|None:
        '''
        Parameters
        -------------------
        category_cut: Selection to be added on top, used for categories

        Returns
        -------------------
        Either:

        - If dataset is not a toy, ROOT dataframe after selection and with Unique identifier attached as uid
        - Otherwise, None
        '''
        if 'toy' in self._sample:
            log.debug(f'Cannot retrieve dataframe for toy sample: {self._sample}')
            return None

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
            process = self._sample,
            ext_cut = cut)

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
            log.debug(f'Extracting toy data for sample {self._sample}')
            arr = self._get_toy_array(sample=self._sample)
            wgt = numpy.ones_like(arr)

            return arr, wgt

        log.debug(f'Extracting data through RDFGetter for sample {self._sample}')

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
    @property
    def rdf_uid(self) -> str|None:
        '''
        Unique identifier of ROOT dataframe after selection
        '''
        return self._rdf_uid
    # ------------------------
    def _data_from_numpy(
            self,
            arr_value : numpy.ndarray,
            arr_weight: numpy.ndarray) -> zdata:
        '''
        We should not use weights if they are all 1s due to problems in zfit

        Parameters
        ------------
        arr_value : Array with values to be fitted
        arr_weight: Array with weights

        Returns
        ------------
        zfit data.
        '''

        arr_is_close = numpy.isclose(arr_weight, 1.0, rtol=1e-5)

        if numpy.all(arr_is_close):
            log.debug('Not using weights for dataset where all weights are 1')
            wgt = None
        else:
            log.debug('Using weights in dataset')
            wgt = arr_weight

        data = zfit.data.from_numpy(obs=self._obs, array=arr_value, weights=wgt)

        return data
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

            data    = self._data_from_numpy(arr_value=arr, arr_weight=wgt)
            return data

        arr, wgt = self._get_array()
        data     = self._data_from_numpy(arr_value=arr, arr_weight=wgt)

        numpy.savez_compressed(data_path, values=arr, weights=wgt)
        self._cache()

        return data
# ------------------------
