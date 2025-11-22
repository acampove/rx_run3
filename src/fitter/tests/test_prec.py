'''
Module with tests for the PRec class
'''

from dmu.workflow.cache import Cache
import mplhep
import pytest
import matplotlib.pyplot as plt

from pathlib                import Path
from dmu.stats.fitter       import Fitter
from dmu.stats              import utilities as sut
from dmu.stats.zfit         import zfit
from dmu.logging.log_store  import LogStore
from rx_common.types        import Trigger
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter
from fitter.prec            import PRec

log=LogStore.add_logger('fitter:test_prec')
#-----------------------------------------------
class Data:
    '''
    Class used to hold data
    '''
    out_dir = '/tmp/tests/fitter/prec'
#-----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_fitter:inclusive_decay_weights' , 10)
    LogStore.set_level('rx_fitter:inclusive_sample_weights', 10)
    LogStore.set_level('rx_fitter:prec'                    , 10)

    plt.style.use(mplhep.style.LHCb2)

    with Cache.turn_off_cache(val=None):
        yield
#-----------------------------------------------
@pytest.mark.parametrize('trig', [Trigger.rk_ee_os, Trigger.rkst_ee_os])
def test_electron(tmp_path : Path, trig : Trigger):
    '''
    Simplest test in electron channel
    '''
    q2bin  = 'jpsi'
    mass   = 'B_const_mass_M'
    label  = r'$M_{DTF}$'

    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc']

    obs=zfit.Space(label, limits=(4500, 6900))

    with RDFGetter.max_entries(value = 100_000),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1= PRec(
            samples =l_samp, 
            trig    =trig, 
            q2bin   =q2bin, 
            d_weight=d_wgt,
            out_dir =f'electron_{trig}')

        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('trig, q2bin, mass', [
    (Trigger.rk_mm_os  , 'jpsi', 'B_const_mass_M'), 
    (Trigger.rk_mm_os  , 'psi2', 'B_const_mass_psi2S_M'), 
    # ---------
    (Trigger.rkst_mm_os, 'jpsi', 'B_const_mass_M'),
    (Trigger.rkst_mm_os, 'psi2', 'B_const_mass_psi2S_M'),
])
def test_muon(tmp_path : Path, trig : Trigger, q2bin : str, mass : str):
    '''
    Simplest test in electron channel
    '''
    label  = r'$M_{DTF}$'

    l_samp = [
        'Bu_JpsiX_mm_eq_JpsiInAcc',
        'Bd_JpsiX_mm_eq_JpsiInAcc',
        'Bs_JpsiX_mm_eq_JpsiInAcc']

    obs=zfit.Space(label, limits=(4500, 6900))

    with RDFGetter.max_entries(value = 100_000),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1= PRec(
            samples =l_samp, 
            trig    =trig, 
            q2bin   =q2bin, 
            d_weight=d_wgt,
            out_dir =f'{trig}_{q2bin}')

        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('trig' , [Trigger.rk_mm_os, Trigger.rkst_mm_os])
@pytest.mark.parametrize('block', range(1, 9))
def test_muon_by_block(tmp_path : Path, trig : Trigger, block : int):
    '''
    Simplest test in electron channel
    '''
    q2bin  = 'jpsi'
    mass   = 'B_const_mass_M'
    label  = r'$M_{DTF}$'

    l_samp = [
        'Bu_JpsiX_mm_eq_JpsiInAcc',
        'Bd_JpsiX_mm_eq_JpsiInAcc',
        'Bs_JpsiX_mm_eq_JpsiInAcc']

    obs  = zfit.Space(label, limits=(4500, 6900))
    d_wgt= {'dec' : 1, 'sam' : 1}

    with sel.custom_selection(d_sel={'block' : f'block == {block}'}),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1= PRec(
            samples =l_samp, 
            trig    =trig, 
            q2bin   =q2bin, 
            d_weight=d_wgt,
            out_dir =f'muon_by_block_{block:03}_{trig}')

        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_reso(q2bin : str, tmp_path : Path):
    '''
    Tests PRec building in resonant bins
    '''
    trig   = Trigger.rk_ee_os

    mass   = {
        'low'     : 'B_Mass_smr',
        'central' : 'B_Mass_smr',
        'jpsi'    : 'B_const_mass_M',
        'psi2'    : 'B_const_mass_psi2S_M',
        'high'    : 'B_Mass_smr'}[q2bin]

    label  = {
        'low'     :r'$M(K^+e^+e^-)$',
        'central' :r'$M(K^+e^+e^-)$',
        'jpsi'    :r'$M_{DTF}(K^+e^+e^-)$',
        'psi2'    :r'$M_{DTF}(K^+e^+e^-)$',
        'high'    :r'$M(K^+e^+e^-)$'}[q2bin]

    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc']

    d_maxe = {
        'low'     : -1,
        'central' : -1,
        'jpsi'    : 50_000,
        'psi2'    : 50_000,
        'high'    : -1}

    obs=zfit.Space(label, limits=(4500, 6900))

    maxe = d_maxe[q2bin]
    with RDFGetter.max_entries(value = maxe),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 0, 'sam' : 0}
        obp_4=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp_4.get_sum(mass=mass, name='PRec_4', obs=obs)

        d_wgt= {'dec' : 0, 'sam' : 1}
        obp_3=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp_3.get_sum(mass=mass, name='PRec_3', obs=obs)

        d_wgt= {'dec' : 1, 'sam' : 0}
        obp_2=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp_2.get_sum(mass=mass, name='PRec_2', obs=obs)

        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_fit():
    '''
    Tests that the PDF is fittable
    '''
    q2bin  = 'high'
    trig   = Trigger.rk_ee_os 
    mass   = 'B_Mass_smr'
    label  = r'$M(K^+e^+e^-)$'
    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc']

    obs=zfit.Space(label, limits=(4500, 6900))
    test = f'reso/fit/{q2bin}'

    d_wgt= {'dec' : 1, 'sam' : 1}
    obp  = PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
    pdf  = obp.get_sum(mass=mass, name='PRec_1', obs=obs)

    if pdf is None:
        raise ValueError('No PDF found')

    nev = zfit.Parameter('nev', 0, 0, 10_000)
    pdf.set_yield(nev)

    sam = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, sam)
    res = obj.fit()

    sut.save_fit(
        data   =sam,
        model  =pdf,
        res    =res,
        plt_cfg=None,
        fit_dir=f'{Data.out_dir}/{test}',
        d_const={})
#-----------------------------------------------
@pytest.mark.parametrize('bdt_cut', [
    'mva.mva_prc > 0.0 && mva.mva_cmb > 0.0',
    'mva.mva_prc > 0.2 && mva.mva_cmb > 0.2',
    'mva.mva_prc > 0.3 && mva.mva_cmb > 0.3',
    'mva.mva_prc > 0.4 && mva.mva_cmb > 0.4',
    'mva.mva_prc > 0.5 && mva.mva_cmb > 0.5',
    'mva.mva_prc > 0.8 && mva.mva_cmb > 0.8',
    'mva.mva_prc > 0.9 && mva.mva_cmb > 0.9'])
@pytest.mark.parametrize('q2bin'  , ['jpsi', 'psi2'])
def test_bdt(q2bin : str, bdt_cut : str, tmp_path : Path):
    '''
    Testing application of BDT cuts
    '''
    obs=zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    mass   = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc']

    d_wgt= {'dec' : 1, 'sam' : 1}
    with Cache.cache_root(tmp_path),\
        sel.custom_selection(d_sel={'bdt' : bdt_cut}):
        obp=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('brem_cut', ['nbrem == 0', 'nbrem == 1', 'nbrem >= 2'])
def test_brem(brem_cut : str):
    '''
    Testing by brem category
    '''
    q2bin  = 'jpsi'
    obs=zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    mass   = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    l_samp = [
            'Bu_JpsiX_ee_eq_JpsiInAcc',
            'Bd_JpsiX_ee_eq_JpsiInAcc',
            'Bs_JpsiX_ee_eq_JpsiInAcc']

    d_wgt= {'dec' : 1, 'sam' : 1}
    with sel.custom_selection(d_sel={'brem' : brem_cut}):
        obp=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_cache():
    '''
    Testing caching of PDF
    '''
    q2bin  = 'jpsi'

    obs    = zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    mass   = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    l_samp = [
            'Bu_JpsiX_ee_eq_JpsiInAcc',
            'Bd_JpsiX_ee_eq_JpsiInAcc',
            'Bs_JpsiX_ee_eq_JpsiInAcc',
            ]

    d_wgt= {'dec' : 1, 'sam' : 1}
    obp=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
    obp.get_sum(mass=mass, name='PRec_1', obs=obs)

    with Cache.turn_off_cache(val = []):
        obp=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt)
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_extended(tmp_path : Path):
    '''
    Testing that PDFs are not extended
    '''
    obs=zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc']

    d_wgt= {'dec' : 1, 'sam' : 1}
    with Cache.cache_root(path = tmp_path):
        obp=PRec(samples=l_samp, trig=trig, q2bin='jpsi', d_weight=d_wgt)
        pdf=obp.get_sum(mass='B_Mass_smr', name='PRec_1', obs=obs)

    if pdf is None:
        raise ValueError('No PDF found')

    assert pdf.is_extended is False
#-----------------------------------------------
@pytest.mark.parametrize('mass' , ['B_M', 'B_Mass', 'B_Mass_smr'])
def test_low_stats(mass : str, tmp_path : Path):
    '''
    Testing with low statistics sample, tight MVA
    '''
    obs    = zfit.Space(mass, limits=(4500, 7000))
    trig   = Trigger.rk_ee_os 
    l_samp = [
        'Bu_JpsiX_ee_eq_JpsiInAcc',
        'Bd_JpsiX_ee_eq_JpsiInAcc',
        'Bs_JpsiX_ee_eq_JpsiInAcc',
        ]

    d_wgt = {'dec' : 1, 'sam' : 1}
    with Cache.cache_root(path=tmp_path),\
        sel.custom_selection(d_sel={'bdt' : 'mva_cmb > 0.9 && mva_prc > 0.9'}):
        obp=PRec(samples=l_samp, trig=trig, q2bin='high', d_weight=d_wgt)
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
