'''
Module used to test EfficiencyScanner class
'''

import os
import numpy
import pytest
import matplotlib.pyplot as plt
import pandas            as pnd

from dmu.logging.log_store import LogStore
from rx_efficiencies.efficiency_scanner import EfficiencyScanner as EffSc

log = LogStore.add_logger('rx_efficiencies:test_efficiency_scanner')
# -----------------------------------
class Data:
    '''
    Class needed to store attributes
    '''
    out_dir = '/tmp/tests/rx_efficiencies/efficiency_scanner'
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_efficiencies:efficiency_scanner', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# -----------------------------------
def _plot_values(
        df  : pnd.DataFrame,
        name:  str) -> None:
    '''
    Plot dataframe as an interpolated mesh
    '''
    pivoted = df.pivot(index='mva_prc', columns='mva_cmb', values='yield')
    plt.imshow(
        pivoted.values,
        origin='lower',
        extent=[
            pivoted.columns.min(), pivoted.columns.max(),
            pivoted.index.min()  , pivoted.index.max()],
        cmap='viridis',
        interpolation='bilinear',
    )

    plt.colorbar(label='z')
    plt.xlabel('MVA${}_{comb}$')
    plt.ylabel('MVA${}_{prec}$')
    plt.xticks(pivoted.columns)
    plt.yticks(pivoted.index)
    plt.tight_layout()

    out_path = f'{Data.out_dir}/{name}.png'
    plt.savefig(out_path)
    plt.close()
# -----------------------------------
def test_scan():
    '''
    Test efficiency scanning
    '''
    cfg = {
            'input' : 
            {
                'sample' : 'Bu_JpsiK_ee_eq_DPC',
                'trigger': 'Hlt2RD_BuToKpEE_MVA',
                'q2bin'  : 'jpsi',
                },
            'variables' : 
            {
                'mva_cmb' : numpy.arange(0.5, 1.0, 0.1),
                'mva_prc' : numpy.arange(0.5, 1.0, 0.1),
                }
            }

    obj = EffSc(cfg=cfg)
    df  = obj.run()

    _plot_values(df=df, name='scan')
# -----------------------------------
