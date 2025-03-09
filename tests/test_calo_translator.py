'''
Module with functions meant to test the row,col <-> x,y translation
'''
import os

import pytest
import pandas             as pnd
import matplotlib.pyplot  as plt
import matplotlib.patches as patches

from dmu.logging.log_store import LogStore
from rx_data               import calo_translator as ctran

log=LogStore.add_logger('rx_data:test_calo_translator')

subdetectors = [
            'Inner',
            'Middle',
            'Outer']
# --------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/rx_data/calo_translator'
# --------------------------------
def _plot_boundaries(ax):
    x1 = -2_000
    y1 = -1_200

    x2 =  2_000
    y2 =  1_200

    height= y2 - y1
    width = x2 - x1

    rectangle = patches.Rectangle((min(x1, x2), min(y1, y2)), width, height, edgecolor='red', facecolor='none', linewidth=2)
    ax.add_patch(rectangle)

    x1 = -1_000
    y1 =   -680

    x2 =  1_000
    y2 =   +680

    height= y2 - y1
    width = x2 - x1

    rectangle = patches.Rectangle((min(x1, x2), min(y1, y2)), width, height, edgecolor='red', facecolor='none', linewidth=2)
    ax.add_patch(rectangle)
# --------------------------------
def _plot_translation(df : pnd.DataFrame, row : int, col : int, det : str, name : str):
    if len(df) != 1:
        print(df)
        raise ValueError(f'Dataframe does not have one and only one element for: {row}/{col}/{name}')

    os.makedirs(f'{Data.out_dir}/{name}', exist_ok=True)

    ax = df.plot.scatter(x='x', y='y', color='blue', title=f'Det={det}; Row={row}; Col={col}')
    ax.set_xlim(-6_000, +6_000)
    ax.set_ylim(-4_000, +4_000)

    _plot_boundaries(ax)

    fname = f'{det}_{row:03}_{col:03}.png'
    plt.savefig(f'{Data.out_dir}/{name}/{fname}')
    plt.close()
# --------------------------------
def _plot_region(df : pnd.DataFrame, det : str, name : str, color : str = 'blue', skip_save : bool = False, ax=None):
    ax=df.plot.scatter(x='x', y='y', color=color, s=0.1, title=det, ax=ax)
    ax.set_xlim(-4_000, +4_000)
    ax.set_ylim(-3_500, +3_500)

    if not skip_save:
        os.makedirs(f'{Data.out_dir}/{name}', exist_ok=True)
        plt.grid()
        plt.savefig(f'{Data.out_dir}/{name}/{det}.png')
        plt.close()

    return ax
# --------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    '''
    This function will execute before the tests
    '''
    os.makedirs(Data.out_dir, exist_ok=True)
    LogStore.set_level('rx_data:calo_translator', 10)
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
def test_plot_xyz():
    '''
    Tests plotting X and Y coodinates
    '''
    df     = ctran.get_data()
    df.x.hist(bins=50, histtype='step', label='x')
    df.y.hist(bins=50, histtype='step', label='y')
    df.z.hist(bins=50, histtype='step', label='z')
    plt.legend()
    plt.savefig(f'{Data.out_dir}/xyz.png')
    plt.close()
# --------------------------------
@pytest.mark.parametrize('det', subdetectors)
def test_scan_dets(det : str):
    '''
    Tests translation from row and column to x and y
    '''
    df = ctran.from_id_to_xy(det=det)

    _plot_region(df, det, name='detector_scan')
# --------------------------------
def test_overlay_detectors():
    '''
    Tests translation from row and column to x and y
    '''

    ax = None
    for det, color in [('Inner', 'red'), ('Middle', 'green'), ('Outer', 'blue')]:
        df = ctran.from_id_to_xy(det=det)
        ax = _plot_region(df, det, name='detector_scan', color=color, skip_save = True, ax=ax)

    plt.grid()
    plt.savefig(f'{Data.out_dir}/overlay_detectors.png')
    plt.close()
# --------------------------------
def test_fully_defined():
    '''
    Tests translation from row and column to x and y
    '''
    x1, y1 = ctran.from_id_to_xy(row=14, col=35, det ='Inner')
    x2, y2 = ctran.from_id_to_xy(row=14, col=35, area=      2)

    assert x1 == x2
    assert y1 == y2
# --------------------------------
