'''
Module holding SampleSplitter class
'''

import numpy
import pandas as pnd

from ROOT                   import RDataFrame # type: ignore
from omegaconf              import DictConfig, OmegaConf
from dmu.logging.log_store  import LogStore
from dmu.rdataframe         import utilities as rut
from dmu.workflow.cache     import Cache     as Wcache

log=LogStore.add_logger('rx_misid:sample_splitter')
# --------------------------------
class SampleSplitter(Wcache):
    '''
    What this returns
    -----------------------
    Class meant to split a dataframe with simulated 
    data it will tag events using the `kind` column as kaon or pion events.

    It adds columns:

    - hadron: pion or kaon, depending on hadron tagging cut from config,
              if candidate is tagged as pion (kaon) then both tracks will be pions (kaons)
    '''
    # --------------------------------
    def __init__(
        self,
        rdf      : RDataFrame,
        cfg      : DictConfig):
        '''
        Parameters
        --------------------
        rdf      : Input dataframe with data to split, It should have attached a `uid` attribute, the unique identifier
        cfg      : omegaconf dictionary with configuration specifying how to split the samples
        '''
        cfg_data = OmegaConf.to_container(cfg, resolve=True)
        super().__init__(
            out_path = 'data_sample_splitter',
            args     = [rdf.uid, cfg_data])

        self._cfg      = cfg
        self._rdf      = rdf
    # --------------------------------
    def _id_from_array(self, array : numpy.ndarray) -> int:
        '''
        Parameters
        -------------
        array: Numpy array with particle IDs

        Returns
        -------------
        Absolute value of particle ID. If multiple IDs are found, raise.
        '''
        array = numpy.abs(array)
        if not numpy.all(array == array[0]):
            log.info(array)
            raise ValueError('Array of IDs contains multiple values')

        particle_id = array[0]

        return particle_id
    # ----------------------
    def _particle_from_simulation(self) -> str|None:
        '''
        Returns
        -------------
        Name of particle associated to MC L1 and L2 leptons, pion, kaon or electron
        If this is not MC, return None.
        If particle is neither raise
        '''
        l_col = [ name.c_str() for name in self._rdf.GetColumnNames() ]

        if 'L1_TRUEID' not in l_col or 'L2_TRUEID' not in l_col:
            log.debug('No simulation hadron ID found, sample is data')
            return None

        d_trueid = self._rdf.AsNumpy(['L1_TRUEID', 'L2_TRUEID'])
        arr_l1id = d_trueid['L1_TRUEID']
        arr_l2id = d_trueid['L2_TRUEID']

        lep1_id  = self._id_from_array(array=arr_l1id)
        lep2_id  = self._id_from_array(array=arr_l2id)

        if lep1_id != lep2_id:
            raise ValueError(f'Lepton IDs differ: {lep1_id} != {lep2_id}')

        if   lep1_id == 211:
            particle = 'pion'
        elif lep1_id == 321:
            particle = 'kaon'
        else:
            raise ValueError(f'Unexpected lepton ID: {lep1_id}')

        log.debug(f'Found hadron: {particle}')

        return particle
    # --------------------------------
    def get_sample(self) -> pnd.DataFrame:
        '''
        For data: Returns pandas dataframe with data split by:

        PassFail: Pass (SS), Fail (OS)
        FailPass: Fail (SS), Pass (OS)
        FailFail: Both electrons fail the PID cut

        Where:
            - SS means same sign as the B and OS is opposite sign
            - These strings are stored in the column "kind"

        For MC: It will only filter by charge and return dataframe without
        PassFail, etc split
        '''
        parquet_path = f'{self._out_path}/sample.parquet'
        if self._copy_from_cache():
            log.warning(f'Cached path found, reusing: {parquet_path}')
            df = pnd.read_parquet(parquet_path, engine='pyarrow')

            return df

        particle = self._particle_from_simulation()
        if particle is not None: # This block runs for simulation
            columns      = self._cfg['branches']
            df           = rut.rdf_to_df(rdf=self._rdf, columns=columns)
            df['hadron'] = particle
            df['kind']   = 'N/A' # in simulation we do not have FF,PF,FP splitting
        else: # This one runs for data
            df_pi = self._get_df(hadron='pion')
            df_kp = self._get_df(hadron='kaon')
            df    = pnd.concat([df_kp, df_pi])

        df.to_parquet(parquet_path, engine='pyarrow')
        self._cache()
        return df
# --------------------------------
