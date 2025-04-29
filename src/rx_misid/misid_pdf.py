'''
Module containing MisID_PDF class
'''
import os
import glob
import zfit
import pandas as pnd

from zfit.core.data         import Data       as zdata
from zfit.core.basepdf      import BasePDF    as zpdf
from zfit.core.interfaces   import ZfitSpace  as zobs
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_misid:misid_pdf')
# ----------------------------------------
class MisIdPdf:
    '''
    Class meant to:

    - Read pandas dataframe with control region weighted data
    - Build a KDE PDF from that data
    - Provide it alongside with the data to the user
    '''
    # ----------------------------------------
    def __init__(self, obs : zobs, q2bin : str):
        '''
        obs : Observable needed for KDE
        q2bin: q2 bin
        '''
        self._obs   = obs
        self._q2bin = q2bin
        self._data  : zdata

        self._bandwidth     = 'isj'
        self._nan_threshold = 0.02
    # ----------------------------------------
    def _preprocess_df(self, df : pnd.DataFrame, sample : str) -> pnd.DataFrame:
        log.debug(f'Preprocessing {sample}')
        df['weight'] = df.apply(lambda x : -abs(x.weight) if x.kind == 'FailFail' else abs(x.weight), axis=1)
        df['sample'] = sample

        self._check_for_nans(df, sample)

        return df
    # ----------------------------------------
    def _check_columns(self, d_df : dict[str,list[str]]) -> None:
        d_col = { sample : df.columns.tolist() for sample, df in d_df.items() }

        last_col = None
        fail     = False
        for l_col in d_col.values():
            if last_col is None:
                last_col = l_col

            if last_col != l_col:
                fail=True

        if not fail:
            log.debug(f'Columns check, passed, columns: {last_col}')
            return

        for sample, l_col in d_col.items():
            log.info(sample)
            log.info(l_col)

        raise ValueError('Columns differ')
    # ----------------------------------------
    def _add_samples(self, d_df : dict[str,pnd.DataFrame]) -> pnd.DataFrame:
        self._check_columns(d_df)

        log.debug('Adding samples')

        l_df_mc = [ df for sample, df in d_df.items() if sample != 'data' ]
        df_mc   = pnd.concat(l_df_mc)
        df_mc['weight'] = - df_mc['weight']

        df_data = d_df['data']
        df      = pnd.concat([df_data, df_mc])

        self._check_for_nans(df, 'merged')

        return df
    # ----------------------------------------
    def _check_for_nans(self, df : pnd.DataFrame, sample : str) -> None:
        nnan = df.isna().sum().sum()
        if nnan == 0:
            log.debug(f'No NaNs found in: {sample}')
            return

        size = len(df)
        if nnan / size < self._nan_threshold:
            log.warning(f'Found {nnan}/{size} in {sample}, cleaning up dataframe')
            df.dropna(inplace=True)
            return

        print(df)
        raise ValueError(f'Found {nnan}/{size} NaNs in {sample}')
    # ----------------------------------------
    def _get_data(self) -> dict[str,pnd.DataFrame]:
        inp_dir = os.environ['MISIDDIR']
        file_wc = f'{inp_dir}/data/*{self._q2bin}.parquet'
        l_path  = glob.glob(file_wc)

        npath   = len(l_path)
        if npath == 0:
            raise ValueError(f'Cannot find any dataset in: {file_wc}')

        log.info(f'Found {npath} datasets')

        d_df    = {}
        for path in l_path:
            fname = os.path.basename(path)
            l_par = fname.split('_')
            sample= l_par[0]

            log.debug(f'Loading: {sample} : {path}')

            df    = pnd.read_parquet(path)

            d_df[sample] = df

        return d_df
    # ----------------------------------------
    def get_data(self) -> pnd.DataFrame:
        '''
        Returns pandas dataframe
        '''
        d_df = self._get_data()
        d_df = { sample : self._preprocess_df(df, sample) for sample, df in d_df.items() }
        df   = self._add_samples(d_df)

        return df
    # ----------------------------------------
    def get_pdf(self) -> zpdf:
        '''
        Return KDE PDF used to model misID
        '''
        df   = self.get_data()

        log.debug('Building data from dataframe')
        data = zfit.data.Data.from_pandas(df=df, obs=self._obs, weights='weight')

        log.debug(f'Building PDF from data with bandwidth {self._bandwidth}')
        pdf  = zfit.pdf.KDE1DimExact(data, bandwidth=self._bandwidth)

        return pdf
# ----------------------------------------
