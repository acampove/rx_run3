'''
Module with tests for the PRec class
'''

import mplhep
import pytest
import matplotlib.pyplot as plt

from pathlib            import Path
from dmu                import LogStore
from dmu.stats          import Fitter
from dmu.stats          import utilities as sut
from dmu.stats          import zfit
from dmu.workflow       import Cache
from rx_common          import Component, Mass, Trigger, Qsq, Channel
from rx_selection       import selection as sel
from rx_data            import RDFGetter
from fitter             import PRec
from fitter.configs import CCbarConf

log=LogStore.add_logger('fitter:test_prec')
#-----------------------------------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_fitter:inclusive_decay_weights' , 10)
    LogStore.set_level('rx_fitter:inclusive_sample_weights', 10)
    LogStore.set_level('rx_fitter:prec'                    , 10)

    plt.style.use(mplhep.style.LHCb2)
#-----------------------------------------------
@pytest.mark.parametrize('trig', [Trigger.rk_ee_os, Trigger.rkst_ee_os])
def test_electron(tmp_path : Path, trig : Trigger):
    '''
    Simplest test in electron channel
    '''
    q2bin   = Qsq.jpsi
    mass    = Mass.bp_dtf_jpsi
    obs     = zfit.Space(
        obs   = mass.latex, 
        label = mass,
        limits=(4500, 6900))

    cfg = CCbarConf.default(channel = Channel.ee, out_dir = tmp_path)

    with RDFGetter.max_entries(value = 100_000),\
         Cache.cache_root(tmp_path):
        obp = PRec(
            cfg   = cfg,
            obs   = obs,
            trig  = trig, 
            q2bin = q2bin)

        obp.get_sum(name = 'ccbar')
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
    label   = r'$M_{DTF}$'
    l_samp  = [ Component.bpjpsixmm, Component.bdjpsixmm, Component.bsjpsixmm ]
    obs     = zfit.Space(label, limits=(4500, 6900))
    out_dir = Path(f'{trig}_{q2bin}')

    with RDFGetter.max_entries(value = 100_000),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1= PRec(
            samples = l_samp, 
            trig    = Trigger(trig), 
            q2bin   = Qsq(q2bin), 
            d_weight= d_wgt,
            out_dir = out_dir)

        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('trig' , [Trigger.rk_mm_os, Trigger.rkst_mm_os])
@pytest.mark.parametrize('block', range(1, 9))
def test_muon_by_block(tmp_path : Path, trig : Trigger, block : int):
    '''
    Simplest test in electron channel
    '''
    q2bin  = Qsq.jpsi 
    mass   = 'B_const_mass_M'
    label  = r'$M_{DTF}$'

    l_samp = [ Component.bpjpsixmm, Component.bdjpsixmm, Component.bsjpsixmm ]
    obs    = zfit.Space(label, limits=(4500, 6900))
    d_wgt  = {'dec' : 1, 'sam' : 1}
    out_dir= Path(f'muon_by_block_{block:03}_{trig}')

    with sel.custom_selection(d_sel={'block' : f'block == {block}'}),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1= PRec(
            samples =l_samp, 
            trig    =trig, 
            q2bin   =q2bin, 
            d_weight=d_wgt,
            out_dir =out_dir)

        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_reso(q2bin : str, tmp_path : Path):
    '''
    Tests PRec building in resonant bins
    '''
    trig   = Trigger.rk_ee_os
    q2bin  = Qsq(q2bin)

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

    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]

    d_maxe = {
        'low'     : -1,
        'central' : -1,
        'jpsi'    : 50_000,
        'psi2'    : 50_000,
        'high'    : -1}

    obs     = zfit.Space(label, limits=(4500, 6900))
    out_dir = Path('reso')

    maxe = d_maxe[q2bin]
    with RDFGetter.max_entries(value = maxe),\
         Cache.cache_root(tmp_path):
        d_wgt= {'dec' : 0, 'sam' : 0}
        obp_4=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt, out_dir = out_dir)
        obp_4.get_sum(mass=mass, name='PRec_4', obs=obs)

        d_wgt= {'dec' : 0, 'sam' : 1}
        obp_3=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt, out_dir = out_dir)
        obp_3.get_sum(mass=mass, name='PRec_3', obs=obs)

        d_wgt= {'dec' : 1, 'sam' : 0}
        obp_2=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt, out_dir = out_dir)
        obp_2.get_sum(mass=mass, name='PRec_2', obs=obs)

        d_wgt= {'dec' : 1, 'sam' : 1}
        obp_1=PRec(samples=l_samp, trig=trig, q2bin=q2bin, d_weight=d_wgt, out_dir = out_dir)
        obp_1.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_fit(tmp_path : Path):
    '''
    Tests that the PDF is fittable
    '''
    q2bin  = Qsq.high
    trig   = Trigger.rk_ee_os 
    mass   = 'B_Mass_smr'
    label  = r'$M(K^+e^+e^-)$'
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]
    obs    = zfit.Space(label, limits=(4500, 6900))
    out_dir= Path(f'reso/fit/{q2bin}')

    d_wgt= {'dec' : 1, 'sam' : 1}
    with Cache.cache_root(path = tmp_path):
        obp = PRec(
            samples = l_samp, 
            trig    = trig, 
            q2bin   = q2bin, 
            d_weight= d_wgt,
            out_dir = out_dir,
        )
        pdf = obp.get_sum(mass=mass, name='PRec_1', obs=obs)

    if pdf is None:
        raise ValueError('No PDF found')

    nev = zfit.Parameter('nev_model_prec_1', 0, 0, 10_000)
    pdf.set_yield(nev)

    sam = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, sam)
    res = obj.fit()

    sut.save_fit(
        data   =sam,
        model  =pdf,
        res    =res,
        plt_cfg=None,
        fit_dir= tmp_path / out_dir,
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
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]

    d_wgt   = {'dec' : 1, 'sam' : 1}
    out_dir = Path('bdt')
    with Cache.cache_root(tmp_path),\
        sel.custom_selection(d_sel={'bdt' : bdt_cut}):
        obp=PRec(
            samples = l_samp, 
            trig    = trig, 
            q2bin   = Qsq(q2bin), 
            d_weight= d_wgt,
            out_dir = out_dir,
        )
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
@pytest.mark.parametrize('brem_cut', ['nbrem == 0', 'nbrem == 1', 'nbrem >= 2'])
def test_brem(brem_cut : str, tmp_path : Path):
    '''
    Testing by brem category
    '''
    q2bin  = 'jpsi'
    obs    = zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    mass   = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]
    d_wgt  = {'dec' : 1, 'sam' : 1}
    out_dir= Path('brem')

    with sel.custom_selection(d_sel={'brem' : brem_cut}),\
        Cache.cache_root(path = tmp_path):
        obp=PRec(
            samples =l_samp, 
            trig    =Trigger(trig), 
            q2bin   =Qsq(q2bin), 
            d_weight=d_wgt,
            out_dir = out_dir,
        )
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_cache(tmp_path : Path):
    '''
    Testing caching of PDF
    '''
    q2bin  = 'jpsi'

    obs    = zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    mass   = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}[q2bin]
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]
    d_wgt  = {'dec' : 1, 'sam' : 1}
    out_dir= Path('cache')

    with Cache.cache_root(path = tmp_path):
        obp=PRec(
            samples = l_samp, 
            trig    = Trigger(trig), 
            q2bin   = Qsq(q2bin), 
            d_weight= d_wgt,
            out_dir = out_dir,
        )
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)

        obp=PRec(
            samples =l_samp, 
            trig    = Trigger(trig), 
            q2bin   = Qsq(q2bin), 
            d_weight=d_wgt,
            out_dir =out_dir,
        )
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
def test_extended(tmp_path : Path):
    '''
    Testing that PDFs are not extended
    '''
    obs    = zfit.Space('mass', limits=(4500, 6000))
    trig   = Trigger.rk_ee_os 
    d_wgt  = {'dec' : 1, 'sam' : 1}
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]
    out_dir= Path('extended')

    with Cache.cache_root(path = tmp_path):
        obp=PRec(
            samples =l_samp, 
            trig    = Trigger(trig), 
            q2bin   = Qsq.jpsi, 
            d_weight=d_wgt,
            out_dir = out_dir,
        )
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
    d_wgt  = {'dec' : 1, 'sam' : 1}
    l_samp = [ Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee ]
    out_dir= Path('low_stats')

    with Cache.cache_root(path=tmp_path),\
        sel.custom_selection(d_sel={'bdt' : 'mva_cmb > 0.9 && mva_prc > 0.9'}):
        obp=PRec(
            samples =l_samp, 
            trig    =trig, 
            q2bin   =Qsq.high, 
            d_weight=d_wgt,
            out_dir = out_dir,
        )
        obp.get_sum(mass=mass, name='PRec_1', obs=obs)
#-----------------------------------------------
