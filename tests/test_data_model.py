'''
This module has tests for the DataModel class
'''
import pytest

from rx_selection          import selection as sel
from rx_data.rdf_getter    import RDFGetter 
from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.stats.zfit        import zfit
from dmu.generic           import utilities as gut
from dmu.stats             import utilities as sut
from dmu.logging.log_store import LogStore
from fitter.data_model     import DataModel

log=LogStore.add_logger('fitter:test_data_model')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('fitter:data_model' , 10)
    LogStore.set_level('rx_data:rdf_getter', 30)
# --------------------------
def test_resonant():
    '''
    Simplest test
    '''

    obs = zfit.Space('B_const_mass_M', limits=(5000, 6000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/electron/data.yaml')

    with RDFGetter.max_entries(value=-1),\
         PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {
        'mass' : '(1)',
        'block': 'block == 1 || block == 2'}):
        dmd = DataModel(
            cfg     = cfg,
            obs     = obs,
            trigger = 'Hlt2RD_BuToKpEE_MVA',
            project = 'rx',
            q2bin   = 'jpsi',
            name    = 'simple')
        pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
def test_rare_electron():
    '''
    Simplest test for rare electron mode
    '''
    q2bin = 'central'

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    with PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {'mass' : '(1)', 'brmp' : 'nbrem != 0'}):
        dmd = DataModel(
            cfg     = cfg,
            obs     = obs,
            trigger = 'Hlt2RD_BuToKpEE_MVA',
            project = 'rx',
            q2bin   = q2bin)
        pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
@pytest.mark.parametrize('tag_cut, observable', [
    ('PROBNN_K < 0.1', 'kpipi'),
    ('PROBNN_K > 0.1', 'kkk'  ),
])
def test_misid_rare(tag_cut : str, observable : str):
    '''
    Test getting model for misid control region
    '''
    q2bin = 'central'

    obs = zfit.Space(f'B_Mass_{observable}', limits=(4500, 7000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='misid/electron/data.yaml')

    l1_in_cr = f'((L1_PROBNN_E < 0.2) || (L1_PID_E < 3.0)) && L1_{tag_cut}'
    l2_in_cr = f'((L2_PROBNN_E < 0.2) || (L2_PID_E < 3.0)) && L2_{tag_cut}'
    out_dir  = f'{cfg.output_directory}/{observable}'
    cfg.output_directory = out_dir

    with PL.parameter_schema(cfg=cfg.model.yields),\
        RDFGetter.default_excluded(names=[]),\
        sel.custom_selection(d_sel = {
        'nobr0' : 'nbrem != 0',
        'pid_l' : f'({l1_in_cr}) && ({l2_in_cr})',
        'mass'  : '(1)',
        'bdt'   : 'mva_cmb > 0.80 && mva_prc > 0.60'}):
        dmd = DataModel(
            name    = observable,
            cfg     = cfg,
            obs     = obs,
            trigger = 'Hlt2RD_BuToKpEE_MVA_noPID',
            project = 'nopid',
            q2bin   = q2bin)
        pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
