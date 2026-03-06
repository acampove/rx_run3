'''
Test for MVAConf class
'''

from fitter import MVAConf

# ----------------------------------------
def test_from_float():
    '''
    Test creating a config from floats
    '''
    cfg = MVAConf(cmb = 0.3, prc = 0.4)
    print('')
    print(cfg)
# ----------------------------------------
def test_from_tuple():
    '''
    Test creating a config from tuple 
    '''
    cfg = MVAConf(
        cmb = (0.2, 0.5), 
        prc = 0.4)
    print('')
    print(cfg)
# ----------------------------------------
