'''
Module with tests for swap calculator class
'''
import os
from importlib.resources     import files

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from rx_data.swp_calculator import SWPCalculator

# ----------------------------------
class Data:
    '''
    Class used to share attributes
    '''

    out_dir : str = '/tmp/rx_data/tests/swap_calculator'
# ----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------------
def _get_df() -> pnd.DataFrame:
    json_path = files('rx_data_data').joinpath('tests/swap_adder/bpd0kpienu.json')
    df = pnd.read_json(json_path)

    return df
# ----------------------------------
def test_simple():
    '''
    Simples test
    '''
    df  = _get_df()

    obj = SWPCalculator(df, d_lep={'L1' : 211, 'L2' : 211}, d_had={'H' : 321})
    df  = obj.get_df()
    df.H_swp.hist(bins=40, range=(1800, 1950) , histtype='step', label='Swapped')
    df.H_org.hist(bins=40, range=(1800, 1950) , histtype='step', label='Original')
    plt.axvline(x=1864, color='r', label='$D_0$')
    plt.grid(False)
    plt.legend()

    plt.savefig(f'{Data.out_dir}/simple.png')
    plt.close('all')
