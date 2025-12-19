'''
Module used to test PrecScales class
'''

import numpy
import pytest

from typing              import Any
from pathlib             import Path
from rx_selection        import selection  as sel
from fitter.prec_scales  import PrecScales

from dmu                 import LogStore
from dmu.workflow        import Cache

log=LogStore.add_logger('fitter:test_prec_scales')
# -----------------------------------
class Data:
    '''
    Data class
    '''
    trigger   = 'Hlt2RD_BuToKpEE_MVA'
    l_mva_cmb = [ f'mva_cmb > {wp:.3f}' for wp in numpy.arange(0.8, 1.0, 0.02) ]
    l_mva_prc = [ f'mva_prc > {wp:.3f}' for wp in numpy.arange(0.8, 1.0, 0.02) ]

    @staticmethod
    def get_seq_wp(min_cmb : float, min_prc : float, step : float) -> list[str]:
        '''
        Will take starting point for both combinatorial and PRec WP, as well as step
        Will tighten one at a time and return list of cuts
        '''
        l_wp = []
        wp_cmb_str = '(1)'
        wp_prc_str = f'mva_prc > {min_prc:.3f}'
        for wp_cmb in numpy.arange(min_cmb, 1, step):
            wp_cmb_str = f'mva_cmb > {wp_cmb:.3f}'
            wp_str     = f'({wp_cmb_str}) && ({wp_prc_str})'
            l_wp.append(wp_str)

        for wp_prc in numpy.arange(min_prc, 1, step):
            wp_prc_str = f'mva_prc > {wp_prc:.3f}'
            wp_str     = f'({wp_cmb_str}) && ({wp_prc_str})'
            l_wp.append(wp_str)

        return l_wp
# -----------------------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This is called before any test
    '''
    LogStore.set_level('rx_fitter:prec_scales', 10)
    LogStore.set_level('rx_efficiencies:efficiency_calculator', 10)
# ----------------------
def _validate_scales(scales : Any) -> None:
    '''
    Validates prec scale and error
    '''
    assert len(scales) == 2
    val, err = scales

    assert isinstance(val, float)
    assert isinstance(err, float)
#-------------------------------
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'high'])
@pytest.mark.parametrize('process', ['bdkstkpiee', 'bpkstkpiee', 'bsphiee'])
def test_all_datasets(q2bin : str, process : str, tmp_path : Path):
    '''
    Tests retrieval of scales between signal and PRec yields
    '''
    signal   = 'bpkpee'
    with Cache.cache_root(path = tmp_path):
        obj    = PrecScales(proc=process, q2bin=q2bin)
        scales = obj.get_scale(signal=signal)

    _validate_scales(scales = scales)
#-------------------------------
@pytest.mark.parametrize('process', ['bdkstkpiee', 'bpkstkpiee', 'bsphiee'])
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'high'])
@pytest.mark.parametrize('mva_cut', Data.get_seq_wp(min_cmb=0.8, min_prc=0.8, step=0.02))
def test_seq_scan_scales(
    mva_cut : str, 
    q2bin   : str, 
    process : str,
    tmp_path: Path) -> None:
    '''
    Tests retrieval of scales between signal and PRec yields, by cutting first on combinatorial and then on PRec
    '''
    signal = 'bpkpee'
    with sel.custom_selection(d_sel={'bdt' : mva_cut}),\
         Cache.cache_root(path = tmp_path):
        obj    = PrecScales(proc=process, q2bin=q2bin)
        scales = obj.get_scale(signal=signal)

    _validate_scales(scales = scales)
#-------------------------------
