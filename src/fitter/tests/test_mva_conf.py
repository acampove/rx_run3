'''
Test for MVAConf class
'''
import pytest

from fitter    import MVAConf
from rx_common import MVA

# ----------------------------------------
def test_from_float():
    '''
    Test creating a config from floats
    '''
    cfg = MVAConf.from_values(cmb = 0.3, prc = 0.4)
    print('')
    print(cfg)
# ----------------------------------------
def test_from_tuple():
    '''
    Test creating a config from tuple 
    '''
    cfg = MVAConf.from_values(
        cmb = (0.2, 0.5), 
        prc = 0.4)
    print('')
    print(cfg)
# ----------------------------------------
def test_str_to_wp():
    '''
    Test transforming string into floats
    '''
    # Single WP for both
    assert MVAConf.str_to_wp(value='030_020', kind=MVA.cmb) == (0.30, None)
    assert MVAConf.str_to_wp(value='030_020', kind=MVA.prc) == (0.20, None)

    # Range for cmb, single for prc
    assert MVAConf.str_to_wp(value='030-050_020', kind=MVA.cmb) == (0.30, 0.50)
    assert MVAConf.str_to_wp(value='030-050_020', kind=MVA.prc) == (0.20, None)

    # Single for cmb, range for prc
    assert MVAConf.str_to_wp(value='030_020-040', kind=MVA.cmb) == (0.30, None)
    assert MVAConf.str_to_wp(value='030_020-040', kind=MVA.prc) == (0.20, 0.40)

    # Range for both
    assert MVAConf.str_to_wp(value='030-050_020-040', kind=MVA.cmb) == (0.30, 0.50)
    assert MVAConf.str_to_wp(value='030-050_020-040', kind=MVA.prc) == (0.20, 0.40)

    # Invalid string raises
    with pytest.raises(ValueError):
        MVAConf.str_to_wp(value='invalid', kind=MVA.cmb)
# ----------------------------------------
