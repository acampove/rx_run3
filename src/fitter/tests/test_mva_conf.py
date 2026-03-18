'''
Test for MVAConf class
'''
import pytest

from fitter    import MVAConf

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
    assert MVAConf.str_to_wp(value='300') == 0.3 

    # Invalid string raises
    with pytest.raises(ValueError):
        MVAConf.str_to_wp(value='2000')
# ----------------------------------------
