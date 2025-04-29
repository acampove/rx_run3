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

        self._bandwidth = 'isj'
    # ----------------------------------------
    def get_pdf(self) -> zpdf:
        '''
        Return KDE PDF used to model misID
        '''
        data = self.get_data()
        pdf  = zfit.pdf.KDE1DimExact(data, bandwidth=self._bandwidth)

        return pdf
    # ----------------------------------------
    def _preprocess_df(self, df : pnd.DataFrame) -> pnd.DataFrame:
        df['weight'] = df.apply(lambda x : -abs(x.weight) if x.kind == 'FailFail' else abs(x.weight), axis=1)

        return df
    # ----------------------------------------
    def _add_samples(self, d_df : dict[str,pnd.DataFrame]) -> pnd.DataFrame:
        df_data = d_df['data']
        del d_df['data']

        l_df_mc = [ df_mc['weight'].apply(lambda x : -x) for df_mc in d_df.values() ]
        l_df    = [df_data] + l_df_mc
        df      = pnd.concat(l_df, axis=0, ignore_index=True)

        return df
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
    def get_data(self) -> zdata:
        '''
        Return zfit data used to make PDF
        '''
        d_df = self._get_data()
        d_df = { sample : self._preprocess_df(df) for sample, df in d_df.items() }
        df   = self._add_samples(d_df)
        data = zfit.data.Data.from_pandas(df=df, obs=self._obs, weights='weight')

        return data
# ----------------------------------------
