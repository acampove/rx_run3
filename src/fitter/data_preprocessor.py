'''
Module holding DataPreprocessor class
'''
from typing  import cast

import numpy
from omegaconf              import ListConfig
from ROOT                   import RDataFrame
from dmu.workflow.cache     import Cache
from dmu.stats.zfit         import zfit
from dmu.generic            import utilities  as gut
from dmu.stats              import utilities  as sut
from dmu.logging.log_store  import LogStore
from zfit.interface         import ZfitData   as zdata
from zfit.interface         import ZfitSpace  as zobs
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
        wgt_cfg : ListConfig|None,
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
        wgt_cfg: OmegaConf's version of a list of strings, each representing a path to a YAML file with configs
                 to extract weights
        cut    : selection that can be added on top. Needed when fits are required in categories, optional
        '''
        self._obs    = obs
        self._sample = sample
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._wgt_cfg= wgt_cfg
        self._rdf    = self._get_rdf(cut=cut)
        self._rdf_uid= None if self._rdf is None else self._rdf.uid

        super().__init__(
            out_path = out_dir,
            obs_name = sut.name_from_obs(obs),
            rdf_uid  = self._rdf_uid)
    # ------------------------
    def _get_rdf(self, cut : str|None) -> RDataFrame:
        '''
        Parameters
        -------------------
        category_cut: Selection to be added on top, used for categories

        Returns
        -------------------
        ROOT dataframe after selection and with Unique identifier attached as uid
        '''
        log.debug('Retrieving dataframe')
        gtr = RDFGetter(
            sample  =self._sample,
            trigger =self._trigger,
            analysis=self._project)

        rdf = gtr.get_rdf(per_file=False)
        uid = gtr.get_uid()

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
    def _add_extra_weights(self, wgt : numpy.ndarray) -> numpy.ndarray:
        '''
        Parameters
        -------------
        wgt: Array of weights already held in ROOT dataframe

        Returns
        -------------
        Optionally augmented weights
        '''
        if self._wgt_cfg is None:
            log.debug('No weight configuration found, using only default weights')
            return wgt

        for cfg_path in self._wgt_cfg:
            wgt *= self._get_extra_weight(path=cfg_path)

        return wgt
    # ------------------------
    def _get_array(self) -> tuple[numpy.ndarray,numpy.ndarray]:
        '''
        Return a tuple of numpy arrays with the observable and weight
        for the sample requested, this array is fully selected
        '''
        log.debug(f'Extracting data through RDFGetter for sample {self._sample}')

        rdf = self._rdf
        if log.getEffectiveLevel() < 20:
            rep = rdf.Report()
            rep.Print()

        name = sut.name_from_obs(obs=self._obs)

        log.debug('Retrieving data')
        arr  = rdf.AsNumpy([name])[name]
        wgt  = rdf.AsNumpy(['weight'])['weight']
        wgt  = self._add_extra_weights(wgt=wgt)

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
        Returns
        ---------------------
        zfit data, should be ready to be used in fit
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
