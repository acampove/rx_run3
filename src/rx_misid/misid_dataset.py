'''
Module holding MisIDDataset class
'''
import os
import copy
import multiprocessing

import pandas                as pnd
import dmu.generic.utilities as gut
from dmu.logging.log_store     import LogStore
from rx_misid.misid_calculator import MisIDCalculator

log=LogStore.add_logger('rx_misid:misid_dataset')
# -------------------------------------------------------
class MisIDDataset:
    '''
    Class intended to provide datasets:

    - Split into pass fail, fail pass, fail fail regions
    - Weighted using PIDCalib weights to _transfer_ them to the signal region
    - For a specific q2bin
    - In a dictionary of dataframes, one per sample, data, MC signal, etc
    '''
    # ---------------------------------
    def __init__(self, q2bin : str, version : str):
        '''
        Parameters:
        -----------------
        q2bin  : All the datasets will be in this q2 bin
        version: Needed to pick configuration
        '''
        self._q2bin     = q2bin
        self._version   = version

        self._sample    : str
        self._out_dir   : str

        self._cfg       = self._get_config()
    # ---------------------------------
    def _get_config(self) -> dict:
        cfg = gut.load_data(package='rx_misid_data', fpath=f'misid_{self._version}.yaml')
        cfg['input']['q2bin' ] = self._q2bin

        return cfg
    # ---------------------------------
    def _make_dataframe(self, sample : str) -> pnd.DataFrame:
        '''
        For a given sample (e.g. Bu_Kee_eq_btosllball05_DPC), through MisIDCalculator,
        get a pandas dataframe with correct weights and return it
        '''
        cfg = copy.deepcopy(self._cfg)
        cfg['input']['sample'] = sample

        obj = MisIDCalculator(cfg=cfg)
        df  = obj.get_misid()

        return df
    # ---------------------------------
    def _cache_data(self, sample : str, df : pnd.DataFrame) -> None:
        '''
        Saves data for later use, if hash found to be the same

        Parameters
        ------------
        sample: Name of sample associated to data, Data, signal, leakage
        df    : Pandas dataframe with entries
        '''
        ana_dir = os.environ['ANADIR']
        mis_dir = self._cfg['output']
        out_dir = f'{ana_dir}/{mis_dir}/data'
        os.makedirs(out_dir, exist_ok=True)

        out_path = f'{out_dir}/{sample}_{self._q2bin}.parquet'
        log.info(f'Saving to: {out_path}')
        df.to_parquet(out_path)
    # ---------------------------------
    def get_data(self) -> dict[str,pnd.DataFrame]:
        '''
        Returns
        ----------------
        Dictionary between component name (signal, control) and dataset in form of dataframe.

        Each pandas dataframe has weighted data representing misID background
        in signal region. The weights are meant to:

        - Be used to _transfer_ the control region to the signal region
        - Scale the leakage from signal etc to the control region
        '''

        d_component = self._cfg['splitting']['samples']
        d_df        = {}
        for component, l_sample in d_component.items():
            nsample  = len(l_sample)
            if nsample == 1:
                [sample] = l_sample

                d_df[component] = self._make_dataframe(sample=sample)
                continue

            # For components that contain multiple samples e.g. Data
            # This can be parallelized
            log.info(f'Using {nsample} processes')
            with multiprocessing.Pool(processes=nsample) as pool:
                l_df = pool.map(self._make_dataframe, l_sample)

            d_df[component] = pnd.concat(l_df)

        for sample, df in d_df.items():
            self._cache_data(sample=sample, df=df)

        return d_df
# ---------------------------------
