'''
Module used to test ModelScales class
'''

import os
import pytest
import matplotlib.pyplot as plt

from dmu.logging.log_store           import LogStore
from rx_efficiencies.model_scales    import ModelScales
from rx_efficiencies.model_scales    import get_proc_labels


log=LogStore.add_logger('rk_extractor:test_scales')
#-------------------------------
def _plot_df(df, trig):
    df = df[df.trig == trig]
    ax = None
    d_proc_lab = get_proc_labels()
    for proc, df_p in df.groupby('kind'):
        ax=df_p.plot(x='year', y='val', yerr='err', ax=ax, label=d_proc_lab[proc], linestyle='none', marker='o')

    os.makedirs('tests/scales/all_datasets', exist_ok=True)

    plt_path = f'tests/scales/all_datasets/{trig}.png'
    log.info(f'Saving to: {plt_path}')
    plt.grid()
    plt.ylabel('Scale')
    plt.ylim(0.0, 0.5)
    plt.xlabel('')
    plt.tight_layout()
    plt.savefig(plt_path)
    plt.close('all')
#-------------------------------
@pytest.mark.parametrize('year'   , ['2024'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA'])
@pytest.mark.parametrize('process', ['bdkskpiee', 'bpk1kpipiee', 'bpk2kpipiee', 'bpkskpiee', 'bsphiee'])
def test_all_datasets(year : str, trigger : str, process : str):
    '''
    Tests retrieval of scales between signal and PRec yields
    '''
    obj      = ModelScales(dset=year, trig=trigger, kind=process)
    val, err = obj.get_scale()

    assert val < 1   # Prec should be smaller than signal
    assert err < val # Error smaller than value
#-------------------------------
