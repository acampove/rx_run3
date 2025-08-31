'''
Module holding DataPreprocessor class
'''
from typing  import cast

import numpy
import pandas   as pnd
from omegaconf                import DictConfig, OmegaConf
from ROOT                     import RDataFrame # type: ignore
from dmu.workflow.cache       import Cache
from dmu.stats.zfit           import zfit
from dmu.stats                import utilities  as sut
from dmu.generic              import utilities  as gut
from dmu.rdataframe           import utilities  as rut
from dmu.pdataframe           import utilities  as put
from dmu.logging.log_store    import LogStore
from zfit.data                import Data
from zfit.interface           import ZfitSpace  as zobs
from rx_data.rdf_getter       import RDFGetter
from rx_selection             import selection  as sel
from rx_misid.sample_splitter import SampleSplitter
from rx_misid.sample_weighter import SampleWeighter

log=LogStore.add_logger('fitter:data_preprocessor')
# ------------------------
# TODO: Shold this class take a dictionary of cuts? 
# Should this be made into a context manager?
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
        wgt_cfg : DictConfig|None,
        is_sig  : bool               = True,
        cut     : dict[str,str]|None = None):
        '''
        Parameters
        --------------------
        out_dir: Directory where caching will happen, with respect to the _cache_root directory
        obs    : zfit observable
        sample : e.g. DATA_24_MagUp...
        trigger: e.g. Hlt2RD...
        project: e.g. rx, nopid
        q2bin  : e.g. central
        wgt_cfg: Dictionary with:
                 key: Representing kind of weight, e.g. pid
                 value: Actual configuration for kind of weight

        is_sig : If true (default) it will pick PID weights for signal region.
                 Otherwise it will use misID control region weights.
        cut    : Selection defining this component category, represented by dictionary where the key are labels
                 and the values are the expressions of the cut
        '''
        self._obs    = obs
        self._sample = sample
        self._trigger= trigger
        self._project= project
        self._q2bin  = q2bin
        self._wgt_cfg= wgt_cfg

        rdf , d_sel, df_ctf  = self._get_rdf(cut=cut)
        self._rdf    = rdf 
        self._d_sel  = d_sel
        self._df_ctf = df_ctf

        self._is_sig = is_sig
        self._rdf_uid= None if self._rdf is None else self._rdf.uid

        super().__init__(
            out_path = out_dir,
            obs_name = sut.name_from_obs(obs=obs),
            obs_range= sut.range_from_obs(obs=obs),
            d_sel    = d_sel,
            is_sig   = is_sig,
            wgt_cfg  = {} if self._wgt_cfg is None else OmegaConf.to_container(self._wgt_cfg, resolve=True),
            rdf_uid  = self._rdf_uid)
    # ------------------------
    def _get_rdf(self, cut : dict[str,str]|None) -> tuple[RDataFrame, dict[str,str], pnd.DataFrame]:
        '''
        Parameters
        -------------------
        category_cut: Selection to be added on top, used for categories.

        Returns
        -------------------
        Tuple with:
           - ROOT dataframe after selection and with Unique identifier attached as uid
           - Dictionary storing selection
           - Pandas dataframe with cutflow
        '''
        log.debug(f'Retrieving dataframe for {self._sample}/{self._trigger}/{self._project}')
        gtr = RDFGetter(
            sample  =self._sample,
            trigger =self._trigger,
            analysis=self._project)

        rdf = gtr.get_rdf(per_file=False)
        uid = gtr.get_uid()

        log.debug(f'Applying selection to {self._sample}/{self._trigger}')

        # overriding only happens for simulation samples
        with sel.custom_selection(d_sel=cut, force_override=True):
            rdf = sel.apply_full_selection(
                rdf     = rdf,
                uid     = uid,
                q2bin   = self._q2bin,
                trigger = self._trigger,
                process = self._sample)

            cfg_sel = sel.selection(process=self._sample, trigger=self._trigger, q2bin=self._q2bin)
            rep = rdf.Report()
            df  = rut.rdf_report_to_df(rep=rep)

        return rdf, cfg_sel, df
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

        for kind, cfg in self._wgt_cfg.items():
            kind    = str(kind)
            new_wgt = self._get_extra_weight(kind=kind, cfg=cfg)
            if new_wgt.shape != wgt.shape:
                raise ValueError(
                    f'''Shapes of original array and {kind} weights differ:
                        {new_wgt.shape} != {wgt.shape}''')

            wgt = wgt * new_wgt

        return wgt
    # ----------------------
    def _get_extra_weight(self, kind : str, cfg : DictConfig) -> numpy.ndarray:
        '''
        Parameters
        -------------
        kind: E.g. PID, Dalitz
        cfg : Configuration needed to extract weights

        Returns
        -------------
        Array of weights
        '''
        if kind == 'PID':
            arr_wgt = self._get_pid_weights(cfg=cfg)
        else:
            raise ValueError(f'Invalid type of weight {kind}')

        return arr_wgt
    # ----------------------
    def _get_pid_weights(self, cfg : DictConfig) -> numpy.ndarray:
        '''
        Parameters
        -------------
        cfg : Dictionary containing configuration for PID, i.e. it should have keys
              `splitting` and `weights` as used in the rx_misid project

        Returns
        -------------
        Array with PID weights
        '''
        log.info(f'Splitting sample: {self._sample}/{self._q2bin}')
        spl   = SampleSplitter(rdf = self._rdf, cfg = cfg.splitting)
        df    = spl.get_sample()

        log.info(f'Getting PID weights for: {self._sample}/{self._q2bin}')
        wgt   = SampleWeighter(
            df    = df,
            cfg   = cfg.weights,
            sample= self._sample,
            is_sig= self._is_sig) # We want weights for the control region
        df  = wgt.get_weighted_data()

        l_wgt = df.attrs['pid_weights']

        return numpy.array(l_wgt)
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
        wgt  = wgt.astype(float)
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
        arr_weight: numpy.ndarray) -> Data:
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

        data = Data.from_numpy(obs=self._obs, array=arr_value, weights=wgt)
        if not isinstance(data, Data):
            raise TypeError('Return type of unbinned data is not Data')

        return data
    # ------------------------
    def get_data(self) -> Data:
        '''
        Returns
        ---------------------
        zfit data, should be ready to be used in fit
        '''
        data_path = f'{self._out_path}/data.npz'
        if self._copy_from_cache():
            log.info(f'Data found cached, loading: {data_path}')
            with numpy.load(data_path) as ifile:
                arr = ifile['values' ]
                wgt = ifile['weights']

            data    = self._data_from_numpy(arr_value=arr, arr_weight=wgt)
            return data

        arr, wgt = self._get_array()
        data     = self._data_from_numpy(arr_value=arr, arr_weight=wgt)

        cuts_path = data_path.replace('.npz' , '.yaml')
        gut.dump_json(data=self._d_sel , path=cuts_path)

        ctfl_path = cuts_path.replace('.yaml',   '.md')
        put.to_markdown(df=self._df_ctf, path=ctfl_path)

        numpy.savez_compressed(data_path, values=arr, weights=wgt)
        self._cache()

        return data
# ------------------------
