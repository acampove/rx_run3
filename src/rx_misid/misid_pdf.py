'''
Module containing MisID_PDF class
'''
import os
from typing              import cast
from importlib.resources import files

import zfit
import yaml
import numpy
import pandas     as pnd
import tensorflow as tf

from zfit.interface            import ZfitData         as zdata
from zfit.interface            import ZfitPDF          as zpdf
from zfit.interface            import ZfitSpace        as zobs
from dmu.generic               import utilities        as gut
from dmu.logging.log_store     import LogStore
from rx_misid.misid_fitter     import MisIDFitter
from rx_misid.misid_dataset    import MisIDDataset
from rx_misid.mc_scaler        import MCScaler

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

        self._data          : zdata
        self._ana_dir       = os.environ['ANADIR']
        self._mis_dir       = f'{self._ana_dir}/misid'
        self._cfg           = gut.load_data(package='rx_misid_data', fpath = 'misid.yaml')

        self._nan_threshold = self._cfg['pdf']['nan_threshold']
        self._l_component   = self._cfg['pdf']['subtract']
        self._d_padding     = self._cfg['pdf']['padding']

        self._d_scale       = self._get_scales()
    # ----------------------------------------
    def _get_scales(self) -> dict[str,float]:
        d_scale = {'data' : 1.0}
        for name in self._l_component:
            [sample]      = self._cfg['splitting']['samples'][name]
            sig_reg       = MisIdPdf.get_signal_cut()

            scl           = MCScaler(q2bin=self._q2bin, sample=sample, sig_reg=sig_reg)
            _, _, scale   = scl.get_scale()
            d_scale[name] = scale

        return d_scale
    # ----------------------------------------
    def _preprocess_df(self, df : pnd.DataFrame, sample : str) -> pnd.DataFrame:
        log.debug(f'Preprocessing {sample}')
        scale        = self._d_scale[sample]

        log.debug(f'Scaling sample {sample} by {scale:.3e}')
        df['weight'] = scale * df['weight']

        df['weight'] = df.apply(lambda x : -abs(x.weight) if x.kind == 'FailFail' else abs(x.weight), axis=1)
        df['sample'] = sample

        self._check_for_nans(df, sample)

        return df
    # ----------------------------------------
    def _check_columns(self, d_df : dict[str,pnd.DataFrame]) -> None:
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
        '''
        Parameters
        -----------------
        d_df: Dictionary with identifiers as keys and dataframes associated to data as values

        Returns
        -----------------
        Dataframe with MC added with negative weights to real data
        '''
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
            log.warning(f'Found {nnan}/{size} NaNs in {sample}, cleaning up dataframe')
            df.dropna(inplace=True)
            return

        log.info(df)
        raise ValueError(f'Found {nnan}/{size} NaNs in {sample}')
    # ----------------------------------------
    def _extend_pdf(self, pdf : zpdf, data : zdata) -> zpdf:
        weights = getattr(data, 'weights', None)
        if not isinstance(weights, tf.Tensor):
            raise ValueError('No weights found for dataset')

        weights = cast(tf.Tensor, weights)
        arr_wgt = weights.numpy()
        nentries= numpy.sum(arr_wgt)

        log.debug(f'Extending PDF with {nentries:.0f} entries')

        nent    = zfit.Parameter('nmisid', nentries, 0, 10 * nentries)
        nent.floating = False

        pdf.set_yield(nent)

        return pdf
    # ----------------------------------------
    def get_data(
            self,
            kind      : str  = 'zfit',
            only_data : bool = False) -> pnd.DataFrame|zdata:
        '''
        Parameters
        ----------------------
        kind      : zfit (default) provides a zfit data object, pandas provides a dataframe
        only_data : If False, it will provide dataset with both:
                - Leakage: With negative, properly normalized weights
                - Data:  With weights of 1

                Otherwise it will provide only data

        Returns:
        ----------------------
        Data used to make KDE
        '''
        obj  = MisIDDataset(q2bin=self._q2bin)
        d_df = obj.get_data(only_data=only_data)

        d_df = { sample : self._preprocess_df(df, sample) for sample, df in d_df.items() }
        df   = self._add_samples(d_df)

        if kind == 'pandas':
            return df

        data = zfit.data.Data.from_pandas(df=df, obs=self._obs, weights='weight')
        data = cast(zdata, data)

        return data
    # ----------------------------------------
    def get_pdf(self, from_fits : bool = False) -> zpdf:
        '''
        Parameters
        -----------------
        from_fits : If true, will use fits to control region, by default False

        Returns
        -----------------
        PDF used to model misID:

        - KDE when done with PassFail approach
        - Parametric when done with fits to control region
        '''
        data = self.get_data(
            kind      = 'zfit',
            only_data = from_fits) # If we fit we need only real data
                                       # If we subtracted backgrounds, we do KDE
        data = cast(zdata, data)

        if not from_fits:
            log.info('Building MisID KDE')
            pdf  = zfit.pdf.KDE1DimISJ(
                data   = data,
                padding= self._d_padding,
                name   = 'MisID')
            pdf  = self._extend_pdf(pdf, data)

            return pdf

        log.info('Retrieving PDF from fits')
        ftr = MisIDFitter(data=data, q2bin=self._q2bin)
        pdf = ftr.get_pdf()

        return pdf
    # ----------------------------------------
    @staticmethod
    def get_signal_cut() -> str:
        '''
        Will return the cut defining the signal region
        '''
        config_path = files('rx_misid_data').joinpath('misid.yaml')
        config_path = str(config_path)
        with open(config_path, encoding='utf-8') as ifile:
            cfg = yaml.safe_load(ifile)

        cut = cfg['splitting']['lepton_tagging']['pass']

        log.info(f'Using signal cut: {cut}')

        cut_l1 = cut.replace('LEP', 'L1')
        cut_l2 = cut.replace('LEP', 'L2')

        return f'({cut_l1}) && ({cut_l2})'
# ----------------------------------------
