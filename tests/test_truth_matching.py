'''
Module needed to test truth matching
'''

import pytest
from rx_selection          import truth_matching as tm
from rx_data.rdf_getter    import RDFGetter
from dmu.logging.log_store import LogStore

# TODO: Add more samples
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
]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:rdf_getter', 40)
# --------------------------
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_nopid(sample : str):
    '''
    Tests truth matching for noPID samples
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='nopid')
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample, kind='bukll')
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    try:
        fin = rdf.Count().GetValue()
    except Exception: 
        raise RuntimeError(f'Cannot apply cut: "{cut}"')

    assert 20 * fin > ini
# --------------------------
@pytest.mark.parametrize('sample', l_sample_ee)
def test_bukee(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='rx')
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
@pytest.mark.parametrize('sample', l_sample_mm)
def test_bukmm(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpMuMu_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='rx')
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample, kind='bukll')
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    assert 20 * fin > ini
# --------------------------
@pytest.mark.parametrize('sample', l_sample_ee)
@pytest.mark.skip(reason='Missing Rk* ntuples')
def test_bdkstee(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_B0ToKpPimEE_MVA'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='rx')
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample, kind='bukll')
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    assert 20 * fin > ini
# --------------------------
