'''
Module with tests for TestConfig class
'''

from rx_tests import TestConfig

# ----------------------
def test_from_package():
    '''
    '''
    cfg = TestConfig.from_package(
        package   = 'rx_tests_data',
        file_path = 'config.yaml')

    assert 'aputilities'          in cfg
    assert cfg.aputilities.splits == 1
