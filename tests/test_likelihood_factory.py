'''
Module meant to test DataFitter class
'''
from dmu.stats.fitter import LogStore
import pytest

from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.stats.zfit     import zfit
from dmu.generic        import utilities  as gut
from rx_data.rdf_getter import RDFGetter
from rx_selection       import selection  as sel
from fitter.likelihood_factory import LikelihoodFactory

# -------------------------------------------
class Data:
    '''
    Used to share attributes
    '''
    d_block_cut = {
        'b12': 'block == 1 || block == 2',
        'b3' : 'block == 3',
        'b4' : 'block == 4',
        'b5' : 'block == 5',
        'b6' : 'block == 6',
        'b78': 'block == 7 || block == 8'}
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:rdf_getter'      , 30)
    LogStore.set_level('rx_misid:sample_weighter', 30)
# -------------------------------------------
def test_config():
    '''
    Tests the config method
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/muon/data.yaml')

    obs = zfit.Space('B_Mass', limits=(4500, 7000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {'bdt' : '(1)'}),\
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
            name   = 'likelihood_factory',
            obs    = obs,
            sample = 'DATA_24_MagDown_24c2',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        cfg = ftr.get_config()

    assert 'selection' in cfg
    assert 'default'   in cfg.selection
    assert 'fit'       in cfg.selection

    sel_def = cfg.selection.default
    sel_fit = cfg.selection.fit

    assert sel_def.keys() == sel_fit.keys()
    assert sel_def.bdt    != sel_fit.bdt
    assert sel_fit.bdt    == '(1)'
# -------------------------------------------
def test_reso_muon():
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/muon/data.yaml')

    obs = zfit.Space('B_Mass', limits=(5000, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {'bdt' : '(1)'}), \
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_MagDown_24c2',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['central'])
def test_rare_muon(q2bin : str):
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/muon/data.yaml')

    obs = zfit.Space('B_Mass', limits=(5000, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('block', ['b12', 'b3', 'b4', 'b5', 'b6', 'b78'])
def test_reso_electron(block : str):
    '''
    Test fitting resonant electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/electron/data.yaml')

    block_cut = Data.d_block_cut[block]

    obs = zfit.Space('B_Mass_smr', limits=(4800, 6000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         RDFGetter.multithreading(nthreads=8),\
         sel.custom_selection(d_sel={
            'block' : block_cut,
            'brm12' : 'nbrem != 0',
            'mass'  : '(1)'}):

        ftr = LikelihoodFactory(
            obs    = obs,
            name   = block,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_rare_electron(q2bin : str):
    '''
    Test fitting rare electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.60 && mva_prc > 0.40'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_high_q2_track():
    '''
    Test fitting rare electron in high q2
    with track based cut and adding brem 0 category
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))

    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel={
            'q2'    : 'q2_track > 14300000 && q2 < 22000000',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = 'high',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
#@pytest.mark.parametrize('q2bin' , ['low', 'central', 'high'])
@pytest.mark.parametrize('q2bin' , ['central'])
@pytest.mark.parametrize('region, tag_cut', [
    ('kkk'  , 'PROBNN_K > 0.1'), 
    ('kpipi', 'PROBNN_K < 0.1')])
def test_rare_misid_electron(q2bin : str, region : str, tag_cut : str):
    '''
    Test building likelihood in misid control regions
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='misid/electron/data.yaml')

    l1_in_cr = f'((L1_PROBNN_E < 0.2) || (L1_PID_E < 3.0)) && L1_{tag_cut}'
    l2_in_cr = f'((L2_PROBNN_E < 0.2) || (L2_PID_E < 3.0)) && L2_{tag_cut}'

    obs = zfit.Space(f'B_Mass_{region}', limits=(4500, 7000))

    with PL.parameter_schema(cfg=cfg.model.yields),\
         RDFGetter.default_excluded(names=[]),\
         sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'mass'  : '(1)',
            'pid_l' : f'({l1_in_cr}) && ({l2_in_cr})',
            'bdt'   : 'mva_cmb > 0.80 && mva_prc > 0.60'}):
        ftr = LikelihoodFactory(
            obs    = obs,
            name   = f'likelihood_factory/{region}',
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA_ext',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
