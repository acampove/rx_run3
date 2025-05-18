'''
Module holding DDFGetter class
'''
# pylint: disable=unnecessary-lambda-assignment

from functools import reduce

import dask
import dask.dataframe as ddf

import uproot
import yaml
import pandas         as pnd
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:rfile:ddfgetter')
# -------------------------------
class DDFGetter:
    '''
    Class used to provide Dask DataFrames from YAML config files. It should handle:

    - Friend trees
    - Multiple files
    '''
    # ----------------------
    def __init__(
            self,
            cfg         : dict      = None,
            config_path : str       = None,
            columns     : list[str] = None):
        '''
        Params
        --------------
        cfg         : Dictionary storing the configuration (optional)
        config_path : Path to YAML configuration file (optional)
        colums      : If passed, will only use these columns to build dataframe
        '''
        self._cfg     = self._load_config(path=config_path) if cfg is None else cfg
        self._columns = columns
    # ----------------------
    def _load_config(self, path : str) -> dict:
        with open(path, encoding='utf-8') as ifile:
            data = yaml.safe_load(ifile)

            return data
    # ----------------------
    def _get_columns_to_keep(self, tree) -> list[str]:
        if self._columns is None:
            return None

        columns       = self._columns + self._cfg['primary_keys']
        columns       = set(columns)
        available     = set(tree.keys())
        columns       = columns & available

        log.debug(f'Keeping only {columns}')

        return list(columns)
    # ----------------------
    def _get_file_df(self, fpath : str) -> pnd.DataFrame:
        with uproot.open(fpath) as file:
            tname   = self._cfg['tree']
            tree    = file[tname]
            columns = self._get_columns_to_keep(tree)
            df      = tree.arrays(columns, library='pd')

        return df
    # ----------------------
    def _get_file_dfs(self, fname : str) -> list[pnd.DataFrame]:
        l_fpath = [ f'{sample_dir}/{fname}'          for sample_dir in self._cfg['samples'] ]
        l_df    = [ self._get_file_df(fpath = fpath) for fpath in l_fpath ]

        return l_df
    # ----------------------
    @dask.delayed
    def _load_root_file(self, fname : str) -> pnd.DataFrame:
        l_primary_key = self._cfg['primary_keys']

        l_df = self._get_file_dfs(fname=fname)
        fun  = lambda df_l, df_r : pnd.merge(df_l, df_r, on=l_primary_key)
        df   = reduce(fun, l_df)


        return df
    # ----------------------
    def get_dataframe(self) -> ddf:
        '''
        Returns dask dataframe
        '''
        l_fname = self._cfg['files']
        l_dfs   = [ self._load_root_file(fname = fname) for fname in l_fname  ]

        output = ddf.from_delayed(l_dfs)

        return output
# -------------------------------
