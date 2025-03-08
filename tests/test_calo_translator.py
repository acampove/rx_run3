'''
Module with functions meant to test the row,col <-> x,y translation
'''
import os

import pytest
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from rx_data               import calo_translator as ctran

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
def test_load():
    '''
    Tests loading data to dataframe
    '''
    log.info('')
    df = ctran.get_data()
    print(df)
    print(df.dtypes)
# --------------------------------
def test_read_xy():
    '''
    Tests reading XY coordinates, given row and column
    '''
    row = 10
    col = 10

    df     = ctran.get_data()
    in_row = df.r == row
    in_col = df.c == col

    df     = df[in_row & in_col]

    print(df)
# --------------------------------
def test_plot_row_col():
    '''
    Plots row and column histograms
    '''
    df     = ctran.get_data()
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
    df     = ctran.get_data()
    df.x.hist(bins=50, histtype='step', label='x')
    df.y.hist(bins=50, histtype='step', label='y')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/xy.png')
    plt.close()
# --------------------------------
@pytest.mark.parametrize('row', [10, 20, 30])
@pytest.mark.parametrize('col', [10, 20, 30])
def test_xy_from_colrow(row : int, col : int):
    '''
    Tests translation from row and column to x and y
    '''
    x, y   = ctran.from_id_to_xy(row, col)
# --------------------------------
