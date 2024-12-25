'''
Module with functions testing AllowedConf class
'''

from rx_kernel import allowed_conf

from ROOT import MessageSvc

MessageSvc.Initialize(-1)

def test_intialize():
    '''
    Will test initialization
    '''
    allowed_conf.Initialize('/home/acampove/Tests/rx_samples')
