'''
Module with functions that test config classes
'''
import pytest

from pathlib     import Path
from dmu         import LogStore
from dmu.generic import utilities as gut
from rx_misid    import MisIDSampleWeights, MisIDSampleSplitting
from rx_common   import Channel, Component, Project
from fitter      import YieldConf
from fitter      import NonParametricConf 
from fitter      import CCbarConf, CombinatorialConf, MisIDConf
from fitter      import ParametricConf
from fitter      import FitModelConf
from fitter      import MisIDFitModel

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
@pytest.mark.parametrize('analysis', ['rk'  , 'rkst'])
@pytest.mark.parametrize('kind'    , ['reso', 'rare'])
@pytest.mark.parametrize('channel' , ['ee'  ,   'mm'])
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
        fpath   = 'misid/rk/ee/combinatorial.yaml')

    CombinatorialConf(**data)
# ----------------------
@pytest.mark.parametrize('analysis', ['rk'  , 'rkst'])
@pytest.mark.parametrize('kind'    , ['reso', 'rare'])
def test_ccbar(analysis : str, kind : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'{kind}/{analysis}/ee/ccbar.yaml')

    CCbarConf(**data)
# ----------------------
@pytest.mark.parametrize('component', [
    Component.bdkstkpiee, 
    Component.bpkstkpiee,
    Component.bsphiee,
    Component.bpkpjpsiee,
    Component.bpkpee])
def test_nonparametric_rare_rk(component : str):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rk/ee/{component}_np.yaml')

    NonParametricConf(**data)
# ----------------------
@pytest.mark.parametrize('component', 
        [Component.bdkstkpiee, 
         Component.bdkstkpipsi2ee,
         Component.bdkstkpijpsiee])
def test_nonparametric_rare_rkst(component : Component):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rkst/ee/{component}_np.yaml')

    NonParametricConf(**data)
# ----------------------
@pytest.mark.parametrize('component', [Component.bpkpee, Component.bpkpmm])
def test_parametric_rk(component : Component):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rk/{component.channel}/{component}.yaml')

    ParametricConf(**data)
# ----------------------
@pytest.mark.parametrize('component', [Component.bdkstkpiee, Component.bdkstkpimm])
def test_parametric_rkst(component : Component):
    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/rkst/{component.channel}/{component}.yaml')

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
@pytest.mark.parametrize('component', [Component.bpkkk, Component.bpkpipi])
@pytest.mark.parametrize('analysis' , [Project.rk])
def test_misid(
    analysis  : Project,
    component : Component):

    data = gut.load_data(
        package = 'fitter_data', 
        fpath   = f'rare/{analysis}/ee/{component}.yaml')

    with MisIDConf.package(name = 'fitter_data'):
        MisIDConf(**data)
# ----------------------
def test_misid_constraint():
    path = Path('misid/rk/ee/data_misid.yaml')

    MisIDFitModel.from_yaml(
        package = 'fitter_data',
        path    = path)
# ----------------------
@pytest.mark.parametrize('channel', [Channel.ee, Channel.mm  ])
@pytest.mark.parametrize('project', [Project.rk, Project.rkst])
def test_full_model(channel: Channel, project : Project):
    path = f'rare/{project}/{channel}/data.yaml'

    FitModelConf.from_yaml(path = path, package = 'fitter_data')
# ----------------------
