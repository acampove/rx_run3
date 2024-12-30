'''
Module with functions testing AllowedConf class
'''

from rx_kernel           import allowed_conf
from rx_common           import utilities as ut

from ROOT import MessageSvc

MessageSvc.Initialize(-1)

def test_intialize():
    '''
    Will test initialization
    '''
    config_dir = ut.get_config_dir()
    allowed_conf.Initialize(config_dir.c_str())
