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
    def __init__(self, sample : str, trigger : str, substr : str = None):
        self._sample  = sample
        self._trigger = trigger
        self._substr  = substr
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
                for trigger_found in d_data[sample]:
                    log.warning(trigger_found)

                raise ValueError(f'Missing trigger {self._trigger} for sample {sample} in {path}')

            l_path_part = d_data[sample][self._trigger]
            if self._substr is not None:
                l_path_part = self._filter_paths(l_path_part)

            l_path += l_path_part

        nfile   = len(l_path)
        if nfile <= 0:
            raise ValueError(f'No files found in: {path}')

        log.debug(f'Using {nfile} files from {path}')

        return { path : self._treename for path in l_path }
    # ------------------------------------
    def _filter_paths(self, l_path : list[str]) -> list[str]:
        l_path_filt = [ path for path in l_path if self._substr in path ]

        npath = len(l_path_filt)
        if npath != 0:
            return l_path_filt

        for path in l_path:
            log.info(path)

        raise ValueError(f'No path could pass filter \"{self._substr}\"')
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
        df['id'] = df.EVENTNUMBER.astype(str) + '_' + df.RUNNUMBER.astype(str)
        df       = df.drop(self._s_keys, axis=1)

        return df
    # ------------------------------------
    def _get_df(self, path : str, columns : set[str]) -> pnd.DataFrame:
        d_file  = self._files_from_yaml(path)
        columns = self._get_intersecting_columns(d_file, columns)
        df      = uproot.concatenate(d_file, expressions=columns | self._s_keys, library='pd')
        df      = self._create_key(df)
        df      = df.set_index('id')
        df      = df[~df.index.duplicated(keep='first')]

        return df
    # ------------------------
    def get_df(self, columns : set[str]) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with a given set of columns
        '''

        df_totl = None
        # Kind = hop, mva, main...
        for kind, path in RDFGetter.samples.items():
            log.info(f'Building chain for {kind} category')
            df_part = self._get_df(path=path, columns=columns)
            if df_totl is None:
                df_totl = df_part
                continue

            df_totl = self._merge_df(df_part, df_totl)

        return df_totl
    # ------------------------
    def _merge_df(self, df_1, df_2):
        same_index = df_1.index.equals(df_2.index)
        if not same_index:
            raise ValueError('Dataframes cannot be merged, they do not have the same indexes')

        df = pnd.merge(df_1, df_2, on='id')

        return df
    # ------------------------
    def get_rdf(self, columns : set[str]) -> RDataFrame:
        '''
        Returns pandas dataframe with a given set of columns
        '''
        df     = self.get_df(columns)
        df     = df.fillna(-999)
        d_nump = { name : df[name].to_numpy() for name in df }
        rdf    = RDF.FromNumpy(d_nump)

        return rdf
# ---------------------------------------------------------------
