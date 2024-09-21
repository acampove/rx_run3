'''
Unit test for LogStore class
'''

import logging

from dmu.logging.log_store import LogStore

# --------------------------------
def test_show():
    '''
    Test for show_loggers
    '''
    LogStore.set_level('sho_1', logging.WARNING)

    LogStore.add_logger('sho_1')
    LogStore.add_logger('sho_2')

    LogStore.show_loggers()
# --------------------------------
def test_messages():
    '''
    Tests each level
    '''
    log = LogStore.add_logger('msg')
    LogStore.set_level('msg', 10)

    log.debug('debug')
    log.info('info')
    log.warning('warning')
    log.error('error')
    log.critical('critical')
# --------------------------------
def test_level():
    '''
    Test for level setting
    '''
    LogStore.add_logger('lvl_10')
    LogStore.add_logger('lvl_20')

    LogStore.set_level('lvl_10', 10)
    LogStore.set_level('lvl_20', 20)

    LogStore.add_logger('lvl_30')
    LogStore.add_logger('lvl_40')

    LogStore.set_level('lvl_30', 30)
    LogStore.set_level('lvl_40', 40)

    LogStore.show_loggers()
# --------------------------------
def main():
    '''
    Tests start here
    '''
    test_messages()
    test_level()
    test_show()
# --------------------------------
if __name__ == '__main__':
    main()
