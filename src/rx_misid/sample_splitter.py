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
    Class meant to split a dataframe

    - With real data into PassFail, FailPass and
      FailFail samples based on a configuration

    - With simulated data it will just tag events
      using the `kind` column as kaon or pion events.

    It also adds columns:

    - hadron: pion or kaon, depending on hadron tagging cut from config,
              if candidate is tagged as pion (kaon) then both tracks will be pions (kaons)
    - kind  : With values PassFail, FailPass, FailFail, only for data

    SS, OS convention:
    -----------------------
    When we refer to PassFail, we mean that the, SS track is the Pass and the OS is the Fail

    **IMPORTANT:** This code drops KpiK and KKpi entries. Therefore number ouf candidates in the output
    is smaller than int he input. This is due to the fact that B->KpiK has a branching fraction ten times
    smaller than B-> Kpipi
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
        self._l_kind   = ['PassFail', 'FailPass', 'FailFail']
    # --------------------------------
    def _get_cuts(self, kind : str, hadron : str) -> tuple[str,str]:
        '''
        This method is only used for real data, not MC

        Parameters
        -----------------
        kind  : PassFail/FailPass/FailFail
        hadron: Either pion or kaon

        Returns
        -----------------
        Cut for the OS or SS tracks, needed to get the data in the corresponding region
        '''
        pass_cut = self._cfg['lepton_tagging']['pass']
        fail_cut = self._cfg['lepton_tagging']['fail']
        hadr_tag = self._cfg['hadron_tagging'][hadron]

        fail_cut = f'({fail_cut}) && ({hadr_tag})'

        lep_ss   = self._cfg['tracks']['ss']
        lep_os   = self._cfg['tracks']['os']

        if   kind == 'PassFail':
            cut_ss   = pass_cut.replace('LEP_', lep_ss + '_')
            cut_os   = fail_cut.replace('LEP_', lep_os + '_')
        elif kind == 'FailPass':
            cut_ss   = fail_cut.replace('LEP_', lep_ss + '_')
            cut_os   = pass_cut.replace('LEP_', lep_os + '_')
        elif kind == 'FailFail':
            cut_ss   = fail_cut.replace('LEP_', lep_ss + '_')
            cut_os   = fail_cut.replace('LEP_', lep_os + '_')
        else:
            raise ValueError(f'Invalid kind: {kind}')

        log.debug(f'Kind: {kind}')
        log.debug(f'SS cut: {cut_ss}')
        log.debug(f'OS cut: {cut_os}')

        return cut_ss, cut_os
    # --------------------------------
    def _get_df(self, hadron : str) -> pnd.DataFrame:
        '''
        Using ROOT dataframe from data, after selection, will:
        - Apply category splitting
        - Build pandas dataframe and return it

        Parameters
        ---------------
        hadron: Either pion or kaon, needed for hadron tagging cut
        '''
        columns      = self._cfg['branches']
        l_df         = []
        for kind in self._l_kind:
            log.info(f'Calculating sample: {kind}/{hadron}')
            rdf            = self._rdf
            cut_os, cut_ss = self._get_cuts(kind=kind, hadron=hadron)

            rdf = rdf.Filter(cut_os, f'OS {kind}')
            rdf = rdf.Filter(cut_ss, f'SS {kind}')

            df = rut.rdf_to_df(rdf=rdf, columns=columns)
            df['kind'] = kind
            l_df.append(df)

        df_tot           = pnd.concat(l_df)
        df_tot['hadron'] = hadron

        return df_tot
    # ----------------------
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
        elif lep1_id ==  11:
            particle = 'electron'
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
        if particle is not None:
            columns      = self._cfg['branches']
            df           = rut.rdf_to_df(rdf=self._rdf, columns=columns)
            df['hadron'] = particle
        else:
            df_pi = self._get_df(hadron='pion')
            df_kp = self._get_df(hadron='kaon')
            df    = pnd.concat([df_kp, df_pi])

        df.to_parquet(parquet_path, engine='pyarrow')
        self._cache()
        return df
# --------------------------------
