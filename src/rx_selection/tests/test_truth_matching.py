'''
Module needed to test truth matching
'''
import re

import pytest
from ROOT                  import RDF
from rx_common.types import Trigger # type: ignore
from rx_selection          import truth_matching as tm
from rx_data.rdf_getter    import RDFGetter
from dmu.logging.log_store import LogStore

# TODO: Add more samples
_RK_MISIDSAMPLES=[
    'Bu_KplKplKmn_eq_sqDalitz_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC',
    'Bu_KplpiplKmn_eq_sqDalitz_DPC',
    'Bu_Kee_eq_btosllball05_DPC',
]

l_sample_kpee = [
    'Bd_JpsiX_ee_eq_JpsiInAcc',
    'Bd_Kstee_eq_btosllball05_DPC',
    'Bs_JpsiX_ee_eq_JpsiInAcc',
    'Bs_phiee_eq_Ball_DPC',
    'Bu_JpsiK_ee_eq_DPC',
    'Bu_JpsiPi_ee_eq_DPC',
    'Bu_JpsiX_ee_eq_JpsiInAcc',
    'Bu_K1ee_eq_DPC',
    'Bu_K2stee_Kpipi_eq_mK1430_DPC',
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_Kstee_Kpi0_eq_btosllball05_DPC',
    'Bu_psi2SK_ee_eq_DPC',
]

l_sample_kstee = [
    'Bu_JpsiX_ee_eq_JpsiInAcc',
    'Bd_JpsiX_ee_eq_JpsiInAcc',
    'Bs_JpsiX_ee_eq_JpsiInAcc',
    # ----
    'Bd_Kstee_eq_btosllball05_DPC',
    'Bd_JpsiKst_ee_eq_DPC',
    'Bs_JpsiKst_ee_eq_DPC',
    # ----
    'Bs_Jpsiphi_ee_eq_CPV_update2012_DPC',
    'Lb_JpsipK_ee_eq_phsp_DPC',
]

l_sample_kpmm = [
    'Bd_Kstmumu_eq_btosllball05_DPC',
    'Bu_JpsiK_mm_eq_DPC',
    'Bu_JpsiPi_mm_eq_DPC',
    'Bu_Kmumu_eq_btosllball05_DPC',
]

l_sample_kstmm = [
    'Bu_JpsiX_mm_eq_JpsiInAcc',
    'Bd_JpsiX_mm_eq_JpsiInAcc',
    'Bs_JpsiX_mm_eq_JpsiInAcc',
    # ----
    'Bd_Kstmumu_eq_btosllball05_DPC',
    'Bd_JpsiKst_mm_eq_DPC',
    'Bs_JpsiKst_mm_eq_DPC',
    # ----
    'Bs_Jpsiphi_mm_eq_CPV_update2012_DPC',
    'Lb_JpsipK_mm_eq_phsp_DPC',
]

log=LogStore.add_logger('rx_selection:test_truth_matching')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:rdf_getter'              , 40)
    LogStore.set_level('rx_selection:truth_matching'     , 10)
    LogStore.set_level('rx_selection:test_truth_matching', 10)
# ----------------------
def _print_values(rdf : RDF.RNode, cut : str, number : int) -> None:
    '''
    This should be ran if zero entries pass the truth matching

    Parameters
    -------------
    rdf   : DataFrame before truth matching
    cut   : Truth matching string
    number: Number of values to print
    '''
    rgx  = r'TMath::Abs\((.*)\)'
    vals = re.findall(rgx, cut)

    if len(vals) != 1:
        raise ValueError(f'Could not find one variable, but: {vals}')

    var = vals[0]

    arr_val = rdf.AsNumpy([var])[var]

    log.info(arr_val[:number])
# ----------------------
def _check_rdf(rdf : RDF.RNode, cut : str) -> None:
    '''
    This should be ran if zero entries pass the truth matching

    Parameters
    -------------
    rdf: DataFrame before truth matching
    cut: Truth matching string
    '''
    l_cut = cut.split('&&')

    log.warning('The following cuts remove all candidates')
    for cut in l_cut:
        res = rdf.Filter(cut)
        val = res.Count().GetValue()
        if val == 0:
            _print_values(rdf=rdf, cut=cut, number=20)
            log.info(cut)
# --------------------------
@pytest.mark.parametrize('sample', _RK_MISIDSAMPLES)
def test_nopid(sample : str):
    '''
    Tests truth matching for noPID samples
    '''
    with RDFGetter.max_entries(value = -1):
        gtr   = RDFGetter(sample=sample, trigger=Trigger.rk_ee_nopid)
        d_rdf = gtr.get_rdf(per_file=True)

    cut = tm.get_truth(sample, kind='bukll')

    fail = False
    for path, rdf in d_rdf.items():
        ini = rdf.Count().GetValue()
        rdf = rdf.Filter(cut, 'truth match')
        try:
            fin = rdf.Count().GetValue()
        except Exception: 
            raise RuntimeError(f'Cannot apply cut: "{cut}"')

        this_fail = 20 * fin < ini
        if this_fail:
            log.error('')
            log.error(path)
            log.error(f'{ini} ---> {fin}')
        else:
            log.debug('')
            log.debug(path)
            log.debug(f'{ini} ---> {fin}')

        fail = fail or this_fail

    assert not fail
# --------------------------
@pytest.mark.parametrize('sample', l_sample_kpee)
def test_bukee(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample, kind='bukll')
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    if fin == 0:
        etype = tm.get_event_type(sample)
        raise ValueError(f'Got no entries for {sample}/{etype} and cut {cut}')

    assert 20 * fin > ini
# --------------------------
@pytest.mark.parametrize('sample', l_sample_kpmm)
def test_bukmm(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpMuMu_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample, kind='bukll')
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    assert 20 * fin > ini
# --------------------------
@pytest.mark.parametrize('sample', l_sample_kstee)
def test_bdkstee(sample : str):
    '''
    Tests truth matching
    '''
    cut = tm.get_truth(sample, kind='bdkstll')
    trigger = 'Hlt2RD_B0ToKpPimEE_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger)
    org = gtr.get_rdf(per_file=False)

    ini = org.Count().GetValue()
    rdf = org.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    if fin == 0:
        _check_rdf(rdf=org, cut=cut)

    assert 20 * fin > ini

    log.info(f'eff={fin/ini=:.3f}')
# --------------------------
@pytest.mark.parametrize('sample', l_sample_kstmm)
def test_bdkstmm(sample : str):
    '''
    Tests truth matching
    '''
    cut = tm.get_truth(sample, kind='bdkstll')

    trigger = 'Hlt2RD_B0ToKpPimMuMu_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger)
    org = gtr.get_rdf(per_file=False)

    ini = org.Count().GetValue()
    rdf = org.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    if fin == 0:
        _check_rdf(rdf=org, cut=cut)

    assert 20 * fin > ini
    log.info(f'eff={fin/ini=:.3f}')
# --------------------------
