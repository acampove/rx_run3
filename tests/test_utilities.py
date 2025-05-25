'''
Module with tests for utility functions
'''
import os
import pytest
import pandas as pnd
import matplotlib.pyplot as plt

from ecal_calibration   import utilities as cut

# -----------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/ecal_calibration/utilities'
# -----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# -----------------------------------------
def _plot_distributions(df : pnd.DataFrame) -> None:
    _plot_vtx(df=df)
    _plot_brem(df=df)
# -----------------------------------------
def _plot_brem(df : pnd.DataFrame) -> None:
    df['nbrem'  ] = df['L1_brem'] + df['L2_brem']

    df['L1_brem'].plot.hist(label='$e^+$', bins=10, alpha=0.3)
    df['L2_brem'].plot.hist(label='$e^-$', bins=10, alpha=0.3)
    df['nbrem'  ].plot.hist(label='both' , bins=10, alpha=0.3)

    plt.legend()
    plt.savefig(f'{Data.out_dir}/brem.png')
    plt.close()
# -----------------------------------------
def _plot_vtx(df : pnd.DataFrame) -> None:
    df['B_END_VX'].plot.hist(label='VX', bins=80, alpha=0.3, range=[-20, +20])
    df['B_END_VY'].plot.hist(label='VY', bins=80, alpha=0.3, range=[-20, +20])
    df['B_END_VY'].plot.hist(label='VY', bins=80, alpha=0.3, range=[-20, +20])

    plt.yscale('log')
    plt.savefig(f'{Data.out_dir}/end_vtx.png')
    plt.close()
# -----------------------------------------
def test_get_ddf():
    '''
    Tests getter of dask dataframe
    '''
    ddf = cut.get_ddf()
    df  = ddf.compute()

    assert len(df) == 10_000

    _plot_distributions(df=df)
# -----------------------------------------
