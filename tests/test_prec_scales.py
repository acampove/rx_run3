'''
Module used to test PrecScales class
'''

import numpy
import pytest

from conftest                    import ScalesData
from dmu.logging.log_store       import LogStore
from dmu.workflow.cache          import Cache
from rx_efficiencies.decay_names import DecayNames as dn
from rx_selection                import selection  as sel
from fitter.prec_scales          import PrecScales

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
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_fitter:prec_scales', 10)
    LogStore.set_level('rx_efficiencies:efficiency_calculator', 10)
#-------------------------------
def _print_selection(
        signal : str,
        prec   : str,
        q2bin  : str) -> bool:
    '''
    Parameters
    --------------

    prec   : Nickname for background sample
    q2bin  : e.g. central
    trigger: HLT2 trigger
    '''
    process= dn.sample_from_decay(signal)
    d_sel  = sel.selection(q2bin=q2bin, process=process, trigger=Data.trigger)
    _print_cuts(d_sel=d_sel)

    process= dn.sample_from_decay(prec  )
    d_sel  = sel.selection(q2bin=q2bin, process=process, trigger=Data.trigger)
    _print_cuts(d_sel=d_sel)

    return False
#-------------------------------
def _print_cuts(d_sel : dict[str,str]) -> None:
    for name, expr in d_sel.items():
        log.info(f'{name:<20}{expr}')
#-------------------------------
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'high'])
@pytest.mark.parametrize('process', ['bdkskpiee', 'bpkskpiee', 'bsphiee'])
def test_all_datasets(q2bin : str, process : str):
    '''
    Tests retrieval of scales between signal and PRec yields
    '''
    signal   = 'bpkpee'
    obj      = PrecScales(proc=process, q2bin=q2bin)
    val, err = obj.get_scale(signal=signal)

    log.info('-' * 20)
    log.info(f'Process: {process}')
    log.info(f'Scale  : {val:.3f}')
    log.info('-' * 20)

    threshold = 1.0
    # We might have large Jpsi leakage in central bin
    if process == 'bpkpjpsiee' and q2bin == 'central':
        threshold = 1.5

    if process != signal:
        # Prec should be smaller than signal
        assert val  < threshold or _print_selection(signal=signal, prec=process, q2bin=q2bin)
    else:
        # If this runs on signal, scale is 1
        assert val ==         1 or _print_selection(signal=signal, prec=process, q2bin=q2bin)

    ScalesData.collect_def_wp(process, '(1)', q2bin, val, err)
#-------------------------------
@pytest.mark.parametrize('process', ['bdkskpiee', 'bpkskpiee', 'bsphiee'])
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'high'])
@pytest.mark.parametrize('mva_cut', Data.get_seq_wp(min_cmb=0.8, min_prc=0.8, step=0.02))
def test_seq_scan_scales(mva_cut : str, q2bin : str, process : str) -> None:
    '''
    Tests retrieval of scales between signal and PRec yields, by cutting first on combinatorial and then on PRec
    '''
    signal = 'bpkpee'
    with sel.custom_selection(d_sel={'bdt' : mva_cut}):
        obj      = PrecScales(proc=process, q2bin=q2bin)
        val, err = obj.get_scale(signal=signal)

    ScalesData.collect_mva_wp(process, mva_cut, q2bin, val, err)
#-------------------------------
