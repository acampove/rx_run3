'''
Module holding RDFGetter class
'''

import fnmatch

import uproot
import pandas as pnd
import yaml

from ROOT                   import RDataFrame, RDF
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------------------
class RDFGetter:
    '''
    Class meant to read ROOT files and return pandas dataframes
    '''
    samples : dict[str,str]
    # ------------------------
    def __init__(self, sample : str, trigger : str):
        self._sample  = sample
        self._trigger = trigger
        self._treename= 'DecayTree'
        self._s_keys  = {'EVENTNUMBER', 'RUNNUMBER'}
    # ------------------------------------
    def _files_from_yaml(self, path : str) -> dict[str,str]:
        with open(path, encoding='utf-8') as ifile:
            d_data = yaml.safe_load(ifile)

        l_path = []
        log.debug('Finding paths')
        for sample in d_data:
            if not fnmatch.fnmatch(sample, self._sample):
                continue

            log.debug(f'    {sample}')

            if self._trigger not in d_data[sample]:
                for key in d_data[sample]:
                    log.warning(key)
                raise ValueError(f'Missing trigger {self._trigger}')

            l_path += d_data[sample][self._trigger]

        nfile   = len(l_path)
        if nfile <= 0:
            raise ValueError(f'No files found in: {path}')

        log.debug(f'Using {nfile} files from {path}')

        return { path : self._treename for path in l_path }
    # ------------------------------------
    def _get_intersecting_columns(self, d_file : dict[str,str], columns : set[str]):
        columns = set(columns)
        for file_name, tree_name in d_file.items():
            with uproot.open(file_name) as ifile:
                tree     = ifile[tree_name]
                branches = set(tree.keys())
                columns &= branches

        if len(columns) == 0:
            print(branches)

        return columns
    # ------------------------------------
    def _create_key(self, df : pnd.DataFrame) -> pnd.DataFrame:
        l_key = list(self._s_keys)
        l_key.sort()

        df['id'] = df.EVENTNUMBER.astype(str) + '_' + df.RUNNUMBER.astype(str)
        df       = df.drop(self._s_keys, axis=1)

        return df
    # ------------------------------------
    def _get_df(self, path : str, columns : set[str]) -> pnd.DataFrame:
        d_file  = self._files_from_yaml(path)
        columns = self._get_intersecting_columns(d_file, columns)
        df      = uproot.concatenate(d_file, expressions=columns | self._s_keys, library='pd')
        df      = self._create_key(df)

        return df
    # ------------------------
    def get_df(self, columns : set[str]) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with a given set of columns
        '''

        df = None
        for kind, path in RDFGetter.samples.items():
            log.debug(f'Building chain for {kind} category')
            df_part = self._get_df(path=path, columns=columns)

            if df is None:
                df = df_part
                continue

            df = pnd.merge(df, df_part, on='id')

        df = df.drop(['id'], axis=1)

        return df
    # ------------------------
    def get_rdf(self, columns : set[str]) -> RDataFrame:
        '''
        Returns pandas dataframe with a given set of columns
        '''
        df     = self.get_df(columns)
        d_nump = { name : df[name].to_numpy() for name in df }
        rdf    = RDF.FromNumpy(d_nump)

        return rdf
# ---------------------------------------------------------------
