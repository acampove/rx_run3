'''
Module holding the Calculator class
'''
import os

import pandas            as pnd
import dask.dataframe    as DaskDataFrame
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_calibration:brem_energy')
# -------------------------------------------------------
class Calculator:
    '''
    Class intended to take a dataset in the form of a Dask DataFrame and:

    - Build a smaller dataframe with:
        0. Scale factor to bring back balance to decay
        1. Energy Bremmstrahlung photon
        2. Type of Bremmstrahlung energy, track based or not.
        3. Position of Bremmstrahlung photon in ROW, COLUMN and AREA (region in ECAL)
        4. Block, describing the period when data was taken.
        5. Proxy for ocupancy.
        ...
    - Build a model that will predict the scale factor
    - Return the model
    '''
    # -------------------------------------------
    def __init__(self, ddf : DaskDataFrame):
        self._df = self._build_feature_dataframe(ddf=ddf)
    # -------------------------------------------
    def _build_feature_dataframe(self, ddf : DaskDataFrame) -> pnd.DataFrame:
        return pnd.DataFrame()
    # -------------------------------------------
    def train(self):
        '''
        Runs training of regressor
        '''
    # -------------------------------------------
    def save(self, out_dir : str) -> None:
        '''

        out_dir (str): Saves all the training artifacts to this directory.
                       It will be made if does not exist
        '''
        os.makedirs(out_dir, exist_ok=True)
# -------------------------------------------------------
