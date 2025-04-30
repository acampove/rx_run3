'''
Module storing data classes needed by tests
Also pytest functions intended to be run after tests
'''
import os

import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

# -----------------------------------
class DataCollector:
    '''
    Class meant to:

    - Collect outputs of tests into dataframes
    '''
    d_df = {}
    # ---------------------------------------
    @staticmethod
    def add_entry(name : str, data : dict):
        '''
        Takes:

        name: Identifier for test, used as key of dictionary of dataframes
        data: Dictionary mapping name of quantity to its value, needed to add row to dataframe
        '''
        if name not in DataCollector.d_df:
            columns = list(data)
            DataCollector.d_df[name] = pnd.DataFrame(columns=columns)

        df=DataCollector.d_df[name]
        df.loc[len(df)] = data
# -----------------------------------
def _plot_scales(df : pnd.DataFrame) -> None:
    print(df)
# -----------------------------------
def pytest_sessionfinish():
    '''
    Runs at the end
    '''
    if 'simple' in DataCollector.d_df:
        df = DataCollector.d_df['simple']
        _plot_scales(df=df)
# -----------------------------------
