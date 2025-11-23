'''
Module used to test EfficiencyScanner class
'''

import pytest
import matplotlib.pyplot as plt
import pandas            as pnd

from pathlib                import Path
from dmu.workflow.cache     import Cache
from dmu                    import LogStore
from rx_efficiencies        import EfficiencyScanner

log = LogStore.add_logger('rx_efficiencies:test_efficiency_scanner')
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    LogStore.set_level('rx_efficiencies:efficiency_scanner', 10)
# -----------------------------------
def _plot_values(
    df      : pnd.DataFrame,
    out_dir : Path,
    name    :  str) -> None:
    '''
    Plot dataframe as an interpolated mesh
    '''
    df['eff'] = 100 * df['eff']

    pivoted = df.pivot(
            index  ='mva_prc',
            columns='mva_cmb',
            values ='eff')

    v1 : float = pivoted.columns.min()
    v2 : float = pivoted.columns.max()
    v3 : float = pivoted.index.min()
    v4 : float = pivoted.index.max()

    plt.imshow(
        pivoted.values,
        origin='lower',
        extent=(v1, v2, v3, v4),
        cmap='viridis',
        interpolation='bilinear',
    )

    plt.title('Efficiencies vs WP')
    plt.colorbar(label='z')
    plt.xlabel('MVA${}_{comb}$')
    plt.ylabel('MVA${}_{prec}$')
    plt.tight_layout()

    out_path = out_dir / f'{name}.png'
    plt.savefig(out_path)
    plt.close()
# -----------------------------------
@pytest.mark.parametrize('sample, q2bin', [
    ('Bu_JpsiK_ee_eq_DPC'        , 'jpsi'   ),
    ('Bu_Kee_eq_btosllball05_DPC', 'low'    ),
    ('Bu_Kee_eq_btosllball05_DPC', 'central'),
    ('Bu_Kee_eq_btosllball05_DPC', 'high'   )])
def test_scan(sample : str, q2bin : str, tmp_path : Path):
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

    with Cache.cache_root(path = tmp_path):
        obj = EfficiencyScanner(cfg=cfg)
        df  = obj.run()

    _plot_values(df=df, name=f'scan_{q2bin}_{sample}', out_dir = tmp_path)
# -----------------------------------
