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
