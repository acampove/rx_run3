'''
Module with tests for ParameterLibrary class
'''

import pytest
from zfit.interface        import ZfitParameter    as zpar
from dmu.stats.zfit        import zfit
from dmu.generic           import utilities        as gut
from dmu.stats.parameters  import ParameterLibrary as PL
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_parameter_library')

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before the tests
    '''
    LogStore.set_level('dmu:stats:parameters', 10)
# ----------------------
def _check_parameter(par : zpar) -> None:
    '''
    Parameters
    -------------
    par: Zfit parameter
    '''
    if not isinstance(par, (zfit.Parameter, zfit.ComposedParameter)):
        type_name = type(par)
        raise TypeError(f'Object is not a parameter but: {type_name}')

    log.info(par)

    val = par.value().numpy() # type: ignore
    assert isinstance(val, float)

    if 'BdKstee' in par.name:
        assert _composed_has_par(name='s_BdKstee'         , par=par)

    if 'BuKstee' in par.name:
        assert _composed_has_par(name='my_preffix_BuKstee', par=par)
# ----------------------
def _composed_has_par(name : str, par : zpar) -> bool:
    '''
    Parameters
    -------------
    name: Name of the component parameter to find
    par : Composed parameter

    Returns
    -------------
    True if `name` parameter exists in `par`
    '''
    s_flt = par.get_params(floating= True)
    s_fix = par.get_params(floating=False)

    s_all = s_flt | s_fix

    for sub_par in s_all:
        if sub_par.name == name:
            return True

    return False
# ------------------------------------
@pytest.mark.parametrize('kind', ['gauss', 'cbl', 'cbr', 'suj'])
def test_print(kind : str):
    '''
    Will test printing parameters
    '''
    PL.print_parameters(kind=kind)
# ------------------------------------
@pytest.mark.parametrize('parameter', ['mu', 'sg', 'ac', 'nc'])
def test_get_values(parameter : str):
    '''
    Will test retrieving values for different parameters of cbl
    '''
    kind = 'cbl'

    x, y, z = PL.get_values(kind=kind, parameter=parameter)

    log.info(f'{parameter:<20}{x:<10.0f}{y:<10.0f}{z:<10.0f}')
# ------------------------------------
@pytest.mark.parametrize('parameter', ['mu', 'sg', 'ac', 'nc'])
def test_values(parameter : str):
    '''
    Will test retrieving values for different parameters of cbl
    '''
    kind = 'cbl'
    x_in = 11111
    y_in = 22222
    z_in = 33333

    with PL.values(kind=kind, parameter=parameter, val=x_in, low=y_in, high=z_in):
        x_out, y_out, z_out = PL.get_values(kind=kind, parameter=parameter)

    assert x_in == x_out
    assert y_in == y_out
    assert z_in == z_out

    x_out, y_out, z_out = PL.get_values(kind=kind, parameter=parameter)

    assert x_in != x_out
    assert y_in != y_out
    assert z_in != z_out
# ------------------------------------
@pytest.mark.parametrize('config', ['scales', 'simultaneous'])
def test_get_yield(config : str) -> None:
    '''
    Tests get_yield method
    '''
    cfg = gut.load_conf(package='dmu_data', fpath=f'tests/stats/parameters/{config}.yaml')
    with PL.parameter_schema(cfg=cfg):
        for parname in cfg:
            parname = str(parname)
            par = PL.get_yield(name=parname)
            _check_parameter(par=par)
