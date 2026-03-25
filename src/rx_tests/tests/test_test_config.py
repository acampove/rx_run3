'''
Module with tests for TestConfig class
'''

from rx_tests import TestConfig

# ----------------------
def test_from_package():
    '''
    Test building config for testing from
    name of project and config file
    '''
    cfg = TestConfig.from_package(
        package   = 'rx_tests_data',
        file_path = 'config.yaml')

    assert 'aputilities'          in cfg.projects
    assert cfg.projects.aputilities.splits == 1
