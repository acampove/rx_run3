'''
Module with functions that test config classes
'''
import pytest


from pathlib     import Path
from dmu         import LogStore
from dmu.generic import utilities as gut
from fitter      import YieldConf
from rx_misid    import MisIDSampleWeights, MisIDSampleSplitting
from fitter      import NonParametricConf 
from fitter      import CCbarConf, CombinatorialConf, MisIDConf
from fitter      import MisIDConstraintConf, ParametricConf
from fitter      import FitModelConf

log=LogStore.add_logger('fitter:test_config')
# ----------------------
@pytest.mark.parametrize('analysis', ['rk', 'rkst'])
def test_yield_conf(analysis : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'model/yields/{analysis}/misid_electron.yaml')

    for name, conf in data.items():
        log.debug(f'Validating: {name}')
        YieldConf(**conf)
# ----------------------
@pytest.mark.parametrize('analysis', ['rk'      , 'rkst'])
@pytest.mark.parametrize('kind'    , ['reso'    , 'rare'])
@pytest.mark.parametrize('channel' , ['electron', 'muon'])
def test_combinatorial_sr(
        analysis : str, 
        channel  : str,
        kind     : str):
    '''
    Test for combinatorial in signal region
    '''
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'{kind}/{analysis}/{channel}/combinatorial.yaml')

    CombinatorialConf(**data)
# ----------------------
def test_combinatorial_cr():
    '''
    Test for combinatorial in misid control region
    '''
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = 'misid/rk/electron/combinatorial.yaml')

    CombinatorialConf(**data)
# ----------------------
@pytest.mark.parametrize('analysis', ['rk'  , 'rkst'])
@pytest.mark.parametrize('kind'    , ['reso', 'rare'])
def test_ccbar(analysis : str, kind : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'{kind}/{analysis}/electron/ccbar.yaml')

    CCbarConf(**data)
# ----------------------
@pytest.mark.parametrize('component', ['bdkstee', 'bukstee', 'bsphiee', 'psi2_leakage', 'jpsi_leakage', 'signal_non_parametric'])
def test_nonparametric_rare_rk(component : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rk/electron/{component}.yaml')

    NonParametricConf(**data)
# ----------------------
@pytest.mark.parametrize('component', ['prec', 'psi2_leakage', 'jpsi_leakage', 'signal_non_parametric'])
def test_nonparametric_rare_rkst(component : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rkst/electron/{component}.yaml')

    NonParametricConf(**data)
# ----------------------
@pytest.mark.parametrize('component', ['signal'])
@pytest.mark.parametrize('analysis' , ['rk', 'rkst'])
@pytest.mark.parametrize('channel'  , ['electron', 'muon'])
def test_parametric(
    analysis  : str,
    channel   : str,
    component : str):

    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/{analysis}/{channel}/{component}.yaml')

    ParametricConf(**data)
# ----------------------
def test_misid_weights():
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = 'model/weights/weights.yaml')

    MisIDSampleWeights(**data)
# ----------------------
def test_misid_splitting():
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = 'model/weights/splitting.yaml')

    MisIDSampleSplitting(**data)
# ----------------------
@pytest.mark.parametrize('component', ['kkk', 'kpipi'])
@pytest.mark.parametrize('analysis' , ['rk'])
def test_misid(
    analysis  : str,
    component : str):

    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/{analysis}/electron/{component}.yaml')

    with MisIDConf.package(name = 'fitter_data'):
        MisIDConf(**data)
# ----------------------
def test_misid_constraint():
    path = Path('misid/rk/electron/data_misid.yaml')

    MisIDConstraintConf.from_yaml(
        package = 'fitter_data',
        path    = path)
# ----------------------
def test_full_model():
    path = 'rare/rkst/muon/data.yaml'

    FitModelConf.from_yaml(path = path, package = 'fitter_data')
# ----------------------
