'''
Module holding MisIDDataset class
'''
import copy

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
    def __init__(self, q2bin : str):
        '''
        Parameters:
        -----------------
        q2bin  : All the datasets will be in this q2 bin
        version: Needed to pick configuration
        '''
        self._q2bin     = q2bin

        self._sample    : str
        self._out_dir   : str

        self._cfg       = self._get_config()
    # ---------------------------------
    def _get_config(self) -> dict:
        cfg = gut.load_data(package='rx_misid_data', fpath=f'misid.yaml')
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
            l_df            = [ self._make_dataframe(sample=sample) for sample in l_sample ]
            d_df[component] = pnd.concat(l_df)

        return d_df
# ---------------------------------
