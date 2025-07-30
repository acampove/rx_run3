'''
Module holding SampleSplitter class
'''

import pandas as pnd
from ROOT                   import RDataFrame

from dmu.logging.log_store  import LogStore
from dmu.rdataframe         import utilities as rut
from dmu.workflow.cache     import Cache     as Wcache

log=LogStore.add_logger('rx_misid:data_sample_splitter')
# --------------------------------
class DataSampleSplitter(Wcache):
    '''
    Class meant to split a dataframe with **real data** into PassFail, FailPass and 
    FailFail samples based on a configuration
    '''
    # --------------------------------
    def __init__(
        self,
        rdf      : RDataFrame,
        sample   : str,
        hadron_id: str,
        is_bplus : bool,
        cfg      : dict):
        '''
        rdf      : Input dataframe with data to split, It should have attached a `uid` attribute, the unique identifier
        sample   : Sample name, e.g. DATA_24_..., needed for output naming
        hadron_id: kaon or pion, needed to extract hadron tagging cuts, when doing data
        is_bplus : True if the sam ple that will be returned will contain B+ mesons, false for B-
        cfg      : Dictionary with configuration specifying how to split the samples
        '''
        super().__init__(
                out_path = f'sample_splitter_{sample}_{hadron_id}_{is_bplus}',
                args     = [rdf.uid, hadron_id, is_bplus, cfg])

        self._sample   = sample
        self._is_bplus = is_bplus
        self._hadron_id= hadron_id
        self._cfg      = cfg
        self._l_kind   = ['PassFail', 'FailPass', 'FailFail']
        self._rdf      = rdf
    # --------------------------------
    def _get_data_cuts(self, kind : str) -> tuple[str,str]:
        '''
        This method is only used for real data, not MC

        Parameters
        -----------------
        kind: PassFail/FailPass/FailFail

        Returns
        -----------------
        Cut for the OS or SS candidates needed to get the data in the corresponding region
        '''
        pass_cut = self._cfg['lepton_tagging']['pass']
        fail_cut = self._cfg['lepton_tagging']['fail']
        hadr_tag = self._cfg['hadron_tagging'][self._hadron_id]

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
    def _get_data_df(self) -> pnd.DataFrame:
        '''
        Using ROOT dataframe from data, after selection, will:
        - Apply category splitting
        - Build pandas dataframe and return it
        '''
        l_df = []
        for kind in self._l_kind:
            log.info(f'Calculating sample: {kind}')
            rdf            = self._rdf
            cut_os, cut_ss = self._get_data_cuts(kind=kind)

            rdf = rdf.Filter(cut_os, f'OS {kind}')
            rdf = rdf.Filter(cut_ss, f'SS {kind}')

            columns = self._cfg['branches']
            df = rut.rdf_to_df(rdf=rdf, columns=columns)
            df['kind'] = kind
            l_df.append(df)

        df_tot = pnd.concat(l_df)
        return df_tot
    # --------------------------------
    def _get_df(self) -> pnd.DataFrame:
        '''
        This method is meant to be a switch between data and MC samples

        Returns
        ---------
        Pandas dataframe with information needed for PID weighting
        '''
        if self._sample.startswith('DATA_'):
            return self._get_data_df()

        columns = self._cfg['branches']
        df = rut.rdf_to_df(rdf=self._rdf, columns=columns)
        df['kind'] = 'full_sample' # MC is not split between PF,FP,FF, column is needed

        return df
    # --------------------------------
    def get_samples(self) -> pnd.DataFrame:
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
            log.warning('Cached object found')
            df = pnd.read_parquet(parquet_path, engine='pyarrow')

            return df

        df           = self._get_df()
        df['hadron'] = self._hadron_id
        df['bmeson'] = 'bplus' if self._is_bplus else 'bminus'

        df.to_parquet(parquet_path, engine='pyarrow')
        self._cache()
        return df
# --------------------------------
