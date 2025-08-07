'''
Module used to test EfficiencyScanner class
'''

import os
import pytest
import matplotlib.pyplot as plt
import pandas            as pnd

from dmu.logging.log_store              import LogStore
from rx_efficiencies.efficiency_scanner import EfficiencyScanner

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
    df['eff'] = 100 * df['eff']

    pivoted = df.pivot(
            index  ='mva_prc',
            columns='mva_cmb',
            values ='eff')

    plt.imshow(
        pivoted.values,
        origin='lower',
        extent=[
            pivoted.columns.min(), pivoted.columns.max(),
            pivoted.index.min()  , pivoted.index.max()],
        cmap='viridis',
        interpolation='bilinear',
    )

    plt.title('Efficiencies vs WP')
    plt.colorbar(label='z')
    plt.xlabel('MVA${}_{comb}$')
    plt.ylabel('MVA${}_{prec}$')
    plt.tight_layout()

    out_path = f'{Data.out_dir}/{name}.png'
    plt.savefig(out_path)
    plt.close()
# -----------------------------------
@pytest.mark.parametrize('sample, q2bin', [
    ('Bu_JpsiK_ee_eq_DPC'        , 'jpsi'   ),
    ('Bu_Kee_eq_btosllball05_DPC', 'low'    ),
    ('Bu_Kee_eq_btosllball05_DPC', 'central'),
    ('Bu_Kee_eq_btosllball05_DPC', 'high'   )])
def test_scan(sample : str, q2bin : str):
    '''
    Test efficiency scanning
    '''
    l_wp = [0.50, 0.80]
    cfg  = {
        'input' :
        {
            'sample' : sample,
            'trigger': 'Hlt2RD_BuToKpEE_MVA',
            'q2bin'  : q2bin,
            },
        'variables' :
        {
            'mva_cmb' : l_wp,
            'mva_prc' : l_wp,
            }
        }

    obj = EfficiencyScanner(cfg=cfg)
    df  = obj.run()

    _plot_values(df=df, name=f'scan_{q2bin}_{sample}')
# -----------------------------------
