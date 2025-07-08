'''
Module meant to hold tests for the DataPreprocessor class
'''
import os
import pytest
import matplotlib.pyplot as plt
from dmu.stats.zfit           import zfit
from dmu.stats                import utilities as sut
from zfit.core.interfaces     import ZfitData  as zdata
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
    rng      = sut.range_from_obs(obs=data.space)

    plt.hist(arr_data, histtype='step', bins=100, range=rng)
    plt.savefig(plt_path)
    plt.close()
# -------------------------------------------------
@pytest.mark.parametrize('sample', ['gauss_toy', 'data_toy'])
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
@pytest.mark.parametrize('sample', ['DATA_24_MagDown_24c1'])
def test_muon_data(sample : str):
    '''
    Tests class with toys
    '''
    obs = zfit.Space('B_Mass', limits=(5180, 6000))

    prp = DataPreprocessor(
        obs    = obs,
        sample = sample,
        trigger= 'Hlt2RD_BuToKpMuMu_MVA',
        project= 'rx',
        q2bin  = 'jpsi')
    dat = prp.get_data()

    _validate_data(data=dat, name=sample)
# -------------------------------------------------
