'''
Module meant to test MisIDDataset class
'''
import os

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.logging.log_store  import LogStore
from rx_misid.misid_dataset import MisIDDataset

log = LogStore.add_logger('rx_misid:test_misid_dataset')
# -----------------------------------------------
class Data:
    '''
    Class used to store attributes
    '''
    plot_dir = '/tmp/tests/rx_misid/misid_dataset/'
# -----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator' , 10)
    LogStore.set_level('rx_misid:misid_dataset'    , 10)

    os.makedirs(Data.plot_dir, exist_ok=True)
# -----------------------------------------------
def _plot_data(
        name : str,
        q2bin: str,
        d_df : dict[str,pnd.DataFrame]) -> None:
    '''
    Parameters
    ------------------
    name: Name of test
    d_df: dictionary between category name and dataframe
    Plots data
    '''
    # B_M_brem_track_2  block   L1_PID_E   L2_PID_E  ...  hadron  log10(L1_TRACK_PT)  log10(L2_TRACK_PT)  bmeson

    plt_dir = f'{Data.plot_dir}/{name}'
    os.makedirs(plt_dir, exist_ok=True)

    _plot_mass(d_df=d_df, name=name, q2bin=q2bin, plt_dir=plt_dir)
    _plot_pide(d_df=d_df, name=name, q2bin=q2bin, plt_dir=plt_dir, var='L1_PID_E')
    _plot_pide(d_df=d_df, name=name, q2bin=q2bin, plt_dir=plt_dir, var='L2_PID_E')
# -----------------------------------------------
def _plot_mass(
        d_df    : dict[str,pnd.DataFrame],
        name    : str,
        q2bin   : str,
        plt_dir : str):
    ax = None
    for category, df in d_df.items():
        ax = df['B_M_brem_track_2'].plot.hist(
                range=(4500,7000),
                bins=40,
                histtype='step',
                label=category,
                ax=ax)

    plt.legend()
    plt.title(f'{q2bin}; {name}')
    plt.savefig(f'{plt_dir}/mass_{q2bin}.png')
    plt.close()
# -----------------------------------------------
def _plot_pide(
        d_df    : dict[str,pnd.DataFrame],
        var     : str,
        name    : str,
        q2bin   : str,
        plt_dir : str):
    ax = None
    for category, df in d_df.items():
        ax = df[var].plot.hist(
                range=(-10,10),
                bins=40,
                histtype='step',
                label=category,
                ax=ax)

    plt.legend()
    plt.title(f'{q2bin}; {name}; {var}')
    plt.savefig(f'{plt_dir}/{var}_{q2bin}.png')
    plt.close()
# -----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_with_leakage(q2bin : str):
    '''
    Returns all components, data and MC
    '''
    dst = MisIDDataset(q2bin=q2bin)
    d_df= dst.get_data(only_data=False)

    assert len(d_df) == 3
    assert 'data'    in d_df
    assert 'signal'  in d_df
    assert 'leakage' in d_df

    _plot_data(
            d_df =d_df,
            q2bin=q2bin,
            name ='with_leakage')
# -----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_only_data(q2bin : str):
    '''
    Simplest test
    '''
    dst = MisIDDataset(q2bin=q2bin)
    d_df= dst.get_data(only_data=True)

    assert len(d_df) == 1
    assert 'data'    in d_df

    _plot_data(
            d_df =d_df,
            q2bin=q2bin,
            name ='no_leakage')
# -----------------------------------------------
