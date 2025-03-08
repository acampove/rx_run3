'''
Module with functions meant to test the row,col <-> x,y translation
'''
import os
from importlib.resources import files

import pytest
import pandas           as pnd
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:test_calo_translator')

# --------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/rx_data/calo_translator'
# --------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    This function will execute before the tests
    '''
    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------------
def _cast_column(column, ctype) -> pnd.Series:
    column = pnd.to_numeric(column, errors='coerce')
    column = column.fillna(-100_000)
    column = column.astype(ctype)

    return column
# --------------------------------
def _get_data() -> pnd.DataFrame:
    data_path = files('rx_data_data').joinpath('brem_correction/coordinates.csv')
    df      = pnd.read_csv(data_path)
    df['x'] = _cast_column(df.x, float)
    df['y'] = _cast_column(df.y, float)
    df['r'] = _cast_column(df.r, int)
    df['c'] = _cast_column(df.c, int)

    df = df[df.x > - 10_000]
    df = df[df.y > - 10_000]

    return df
# --------------------------------
def test_load():
    '''
    Tests loading data to dataframe
    '''
    log.info('')
    df = _get_data()
    print(df)
    print(df.dtypes)
# --------------------------------
def test_read_xy():
    '''
    Tests reading XY coordinates, given row and column
    '''
    row = 10
    col = 10

    df     = _get_data()
    in_row = df.r == row
    in_col = df.c == col

    df     = df[in_row & in_col]

    print(df)
# --------------------------------
def test_plot_row_col():
    '''
    Plots row and column histograms
    '''
    df     = _get_data()
    df.c.hist(bins=64, histtype='step', linestyle='-', label='column')
    df.r.hist(bins=64, histtype='step', linestyle=':', label='row')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/row_col.png')
    plt.close()
# --------------------------------
def test_plot_xy():
    '''
    Tests plotting X and Y coodinates
    '''
    df     = _get_data()
    df.x.hist(bins=50, histtype='step', label='x')
    df.y.hist(bins=50, histtype='step', label='y')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/xy.png')
    plt.close()
# --------------------------------
