'''
This module has tests for the DataModel class
'''
import pytest

from rx_common.types       import Trigger
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
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('fitter:data_model' , 10)
    LogStore.set_level('rx_data:rdf_getter', 30)
# --------------------------
@pytest.mark.parametrize('kind', ['reso/muon'])
def test_resonant(kind : str):
    '''
    Simplest test
    '''

    obs = zfit.Space('B_const_mass_M', limits=(5000, 6000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  =f'{kind}/data.yaml')

    with RDFGetter.max_entries(value=-1),\
         PL.parameter_schema(cfg=cfg.model.yields),\
         sel.custom_selection(d_sel = {
        'mass' : '(1)',
        'block': 'block == 1'}):
        dmd = DataModel(
            cfg     = cfg,
            obs     = obs,
            trigger = Trigger.rk_ee_os,
            q2bin   = 'jpsi')
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
            trigger = Trigger.rk_ee_os,
            q2bin   = q2bin)
        pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
@pytest.mark.parametrize('observable', ['kpipi', 'kkk'])
@pytest.mark.parametrize('q2bin'     , ['low', 'central', 'high'])
def test_misid_rare(observable : str, q2bin : str):
    '''
    Test getting model for misid control region
    '''
    obs = zfit.Space(f'B_Mass_{observable}', limits=(4500, 7000))
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='misid/rk/electron/data_misid.yaml')

    out_dir  = f'{cfg.output_directory}/{observable}'
    cfg.output_directory = out_dir

    with PL.parameter_schema(cfg=cfg.model.yields),\
        RDFGetter.default_excluded(names=[]),\
        sel.custom_selection(d_sel = {
        'nobr0' : 'nbrem != 0',
        'mass'  : '(1)',
        'bdt'   : 'mva_cmb > 0.80 && mva_prc > 0.60'}):
        dmd = DataModel(
            name    = observable,
            cfg     = cfg,
            obs     = obs,
            trigger = Trigger.rk_ee_nopid,
            q2bin   = q2bin)
        pdf = dmd.get_model()

    sut.print_pdf(pdf)
# --------------------------
