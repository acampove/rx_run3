'''
This module is used by pytest to _inject_ fixtures in the tests
'''
import logging
import pytest

from dmu.logging.log_store import LogStore

# ---------------------------------------
def pytest_configure(config : pytest.Config) -> None:
    '''
    Runs before all tests, needed to do global initialization. e.g. logging level setting
    '''
    _ = config

    logging.getLogger("PIL.PngImagePlugin").setLevel(logging.WARNING)
    logging.getLogger("matplotlib.font_manager").setLevel(logging.WARNING)

    LogStore.set_level('rx_calibration:pizerodb', 10)
# ---------------------------------------
