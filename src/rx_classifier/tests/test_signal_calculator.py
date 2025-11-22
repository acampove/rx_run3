'''
This module contains tests for SignalCalculator class
'''
import pandas            as pnd
import matplotlib.pyplot as plt
import pytest

from dmu.workflow                    import Cache
from pathlib                         import Path
from dmu.generic                     import utilities as gut
from rx_classifier.signal_calculator import SignalCalculator

# -----------------------------------
def _plot_values(
    df      : pnd.DataFrame,
    out_dir : Path,
    name    : str) -> None:
    '''
    Plot dataframe as an interpolated mesh
    '''
    pivoted = df.pivot(
            index  ='mva_prc',
            columns='mva_cmb',
            values ='sig'    )

    plt.imshow(
        pivoted.values,
        origin='lower',
        extent=(
            pivoted.columns.min(), pivoted.columns.max(),
            pivoted.index.min()  , pivoted.index.max()),
        cmap='viridis',
        interpolation='bilinear',
    )

    plt.title('Signal yield vs WP')
    plt.xlabel('MVA${}_{comb}$')
    plt.ylabel('MVA${}_{prec}$')
    plt.tight_layout()

    out_path = out_dir / f'{name}.png'
    plt.savefig(out_path)
    plt.close()
# --------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str, tmp_path : Path):
    '''
    Simplest test
    '''
    cfg = gut.load_data(package='rx_classifier_data', fpath='optimization/scanning.yaml')

    with Cache.cache_root(path = tmp_path):
        cal = SignalCalculator(cfg=cfg, q2bin=q2bin)
        df  = cal.get_signal(control=100_000)

    _plot_values(df = df, name = f'simple_{q2bin}', out_dir = tmp_path)
# --------------------------------
