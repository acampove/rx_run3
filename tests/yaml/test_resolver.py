'''
Tests for YamlResolver class
'''
import pytest

from dmu.yaml.resolver     import Resolver
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:yaml_resolver')

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:yaml:resolver', 10)
# ----------------------
def _get_config(kind : str) -> dict[str|int,str]:
    '''
    Returns
    -------------
    Dictionary with configuration
    '''
    data      = {}
    if kind == 'simple':
        data['d'] = 'nested'
        data['a'] = 'something {d}'
        data['b'] = 'here' 
        data['c'] = '{a} {b}' 

    if kind == 'recursive_1':
        data['d'] = 'nested'
        data['a'] = 'something {a}'
        data['b'] = 'here' 
        data['c'] = '{a} {b}' 

    if kind == 'recursive_2':
        data['d'] = 'nested {a}'
        data['a'] = 'something {d}'
        data['b'] = 'here' 
        data['c'] = '{a} {b}' 


    return data
# ----------------------
@pytest.mark.parametrize('kind', ['simple', 'recursive_1', 'recursive_2'])
@pytest.mark.timeout(1)
def test_simple(kind : str):
    cfg = _get_config(kind=kind)

    yrs = Resolver(cfg=cfg)

    assert 'c' in yrs

    assert 'missing' not in yrs

    if kind == 'simple':
        assert yrs['c'] == 'something nested here'
        return

    if kind in ['recursive_1', 'recursive_2']:
        with pytest.raises(ValueError):
            yrs['a']

        return

    raise ValueError(f'Invalid kind: {kind}')

