'''
Module where config holder is tested
'''

from rx_kernel.config_holder import ConfigHolder

from ROOT import MessageSvc

MessageSvc.Initialize(-1)
# -----------------------------------
def test_default():
    '''
    Simplest test of default constructor
    '''
    ch = ConfigHolder()
    ch.Print()
# -----------------------------------
# -----------------------------------
def test_string():
    '''
    Simplest test of default constructor
    '''
    cfg = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'LPT',
            'q2bin'   : 'central',
            'year'    : '24',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL'}

    ch = ConfigHolder(cfg=cfg)
    ch.Print()
# -----------------------------------
