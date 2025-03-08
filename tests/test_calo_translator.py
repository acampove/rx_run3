'''
Module with functions meant to test the row,col <-> x,y translation
'''

from importlib.resources import files

import pandas           as pnd
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:test_calo_translator')

# --------------------------------
def _get_data() -> pnd.DataFrame:
    data_path = files('rx_data_data').joinpath('brem_correction/coordinates.csv')
    df      = pnd.read_csv(data_path)
    df['x'] = df.x.astype(float)
    df['y'] = df.y.astype(float)
    df['z'] = df.z.astype(float)
    df['r'] = df.r.astype(int)
    df['c'] = df.c.astype(int)

    df = df[df.x > - 10_000]
    df = df[df.y > - 10_000]

    return df
# --------------------------------
def test_load():
    '''
    Tests loading data to dataframe
    '''
    log.info('')
    _get_data()
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
    df.c.hist(bins=50, label='column')
    df.r.hist(bins=50, label='row')
    plt.legend()
    plt.show()
# --------------------------------
def test_plot_xyz():
    '''
    Tests plotting X, Y and Z coodinates
    '''
    df     = _get_data()
    df.x.hist(bins=50, histtype='step', label='x')
    df.y.hist(bins=50, histtype='step', label='y')
    df.z.hist(bins=50, histtype='step', label='z')
    plt.legend()
    plt.show()
# --------------------------------
