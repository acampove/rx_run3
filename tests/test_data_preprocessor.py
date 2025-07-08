'''
Module meant to hold tests for the DataPreprocessor class
'''
import os
import pytest
import matplotlib.pyplot as plt
from dmu.stats.zfit           import zfit
from zfit.core.interfaces     import ZfitData as zdata
from fitter.data_preprocessor import DataPreprocessor

# -------------------------------------------------
class Data:
    '''
    Meant to hold shared attributes
    '''
    out_dir = '/tmp/tests/fitter/data_preprocessor'
    os.makedirs(out_dir, exist_ok=True)
# -------------------------------------------------
def _validate_data(data : zdata, name : str) -> None:
    '''
    Makes validation plots from zfit data
    '''
    plt_path = f'{Data.out_dir}/{name}.png'

    arr_data = data.numpy()
    plt.hist(arr_data, bins=50, range=(4500, 7000))
    plt.savefig(plt_path)
    plt.close()
# -------------------------------------------------
@pytest.mark.parametrize('sample', ['gauss_toy'])
def test_toy(sample : str):
    '''
    Tests class with toys
    '''
    obs = zfit.Space('x', limits=(4500, 7000))

    prp = DataPreprocessor(
        obs    =obs,
        sample =sample,
        trigger='',
        project='',
        q2bin  ='')
    dat = prp.get_data()

    _validate_data(data=dat, name=sample)
# -------------------------------------------------
