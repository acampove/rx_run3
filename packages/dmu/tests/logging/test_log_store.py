'''
Unit test for LogStore class
'''

import logging
from dataclasses import dataclass

import pytest
from dmu.logging.log_store import LogStore

# --------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
    l_backend = ['logging']
    l_level   = [5, 10, 20, 30, 40, 50]
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
def test_messages():
    '''
    Tests each level
    '''

    log = LogStore.add_logger('messages')
    LogStore.set_level('messages', 5)

    log.info('\n')
    log.verbose('verbose')
    log.debug('debug')
    log.info('info')
    log.warning('warning')
    log.error('error')
    log.critical('critical')
# --------------------------------
@pytest.mark.parametrize('level'  , Data.l_level)
def test_level(level : int):
    '''
    Test for level setting
    '''
    name = f'level_{level}'

    log=LogStore.add_logger(name)
    LogStore.set_level(name, level)

    assert log.getEffectiveLevel() == level

    LogStore.show_loggers()
# --------------------------------
def test_exists_ok_true():
    '''
    Tests exists_ok flag with value of True
    '''
    log_1  = LogStore.add_logger('exists_ok_true')
    log_2  = LogStore.add_logger('exists_ok_true', exists_ok=True)

    assert log_1 is log_2
# --------------------------------
def test_exists_ok_default():
    '''
    Tests exists_ok flag with default value
    '''
    LogStore.add_logger('exists_ok_default')
    with pytest.raises(ValueError):
        LogStore.add_logger('exists_ok_default')
# --------------------------------
def test_context_logger():
    '''
    Tests level context manager
    '''
    log = LogStore.add_logger('context')

    print(20 * '-')
    log.verbose('Verbose message')
    log.debug('Debug message')
    log.info('Info message')
    log.warning('Warning message')
    log.error('Error message')
    print(20 * '-')
    with LogStore.level('context', 1):
        log.verbose('Verbose message')
        log.debug('Debug message')
        log.info('Info message')
        log.warning('Warning message')
        log.error('Error message')
    print(20 * '-')
    with LogStore.level('context', 30):
        log.verbose('Verbose message')
        log.debug('Debug message')
        log.info('Info message')
        log.warning('Warning message')
        log.error('Error message')
    print(20 * '-')
    log.verbose('Verbose message')
    log.debug('Debug message')
    log.info('Info message')
    log.warning('Warning message')
    log.error('Error message')
# --------------------------------
