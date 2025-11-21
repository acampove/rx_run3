'''
Unit test for LogStore class
'''

import logging
from dataclasses import dataclass

import pytest
from ap_utilities.logging.log_store import LogStore

# --------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
    l_backend = ['logging', 'logzero']
    l_level   = [10, 20, 30, 40, 50]
# --------------------------------
@pytest.mark.parametrize('backend', Data.l_backend)
def test_show(backend : str):
    '''
    Test for show_loggers
    '''
    LogStore.backend = backend

    name_war = f'show_warning_{backend}'
    name_def = f'show_default_{backend}'

    LogStore.set_level(name_war, logging.WARNING)

    LogStore.add_logger(name_war)
    LogStore.add_logger(name_def)

    LogStore.show_loggers()
# --------------------------------
@pytest.mark.parametrize('backend', Data.l_backend)
def test_messages(backend : str):
    '''
    Tests each level
    '''
    LogStore.backend = backend

    name = f'messages_{backend}'
    log = LogStore.add_logger(name)
    LogStore.set_level(name, 10)

    log.debug('debug')
    log.info('info')
    log.warning('warning')
    log.error('error')
    log.critical('critical')
# --------------------------------
@pytest.mark.parametrize('backend', Data.l_backend)
@pytest.mark.parametrize('level'  , Data.l_level)
def test_level(backend : str, level : int):
    '''
    Test for level setting
    '''
    LogStore.backend = backend

    name = f'level_{backend}_{level}'

    LogStore.add_logger(name)
    LogStore.set_level(name, level)

    LogStore.show_loggers()
# --------------------------------
@pytest.mark.parametrize('level', Data.l_level)
def test_logzero(level : int):
    '''
    Tests logzero
    '''
    LogStore.backend = 'logzero'

    name = f'logzero_{level}'
    log  = LogStore.add_logger(name)
    LogStore.set_level(name, level)

    print(30 * '-')
    print(f'Level: {level}')
    print(30 * '-')
    log.debug('debug')
    log.info('info')
    log.warning('warning')
    log.error('error')
    log.critical('critical')
    print(30 * '-')
