'''
Module containing tests for TupleHolder
'''

from rx_kernel.tuple_holder  import TupleHolder
from rx_kernel.config_holder import ConfigHolder

# -------------------------
def _get_config_holder():
    cfg = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'LPT',
            'q2bin'   : 'central',
            'year'    : '18',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL'}

    return ConfigHolder(cfg=cfg)
# -------------------------
def test_default():
    '''
    Test for default constructor
    '''
    obj = TupleHolder()
    obj.PrintInline()
# -------------------------
def test_2018():
    '''
    Will test with 2018 configuration
    '''
    ch  = _get_config_holder()
    obj = TupleHolder(ch,'')
    obj.PrintInline()
# -------------------------
