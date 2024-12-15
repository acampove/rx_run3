'''
Tests for RFPrinter class
'''

import os
import pytest

from dmu.rfile.rfprinter import RFPrinter

# -------------------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    file_path_simple : str
    file_path_fail   : str
# -------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize() -> None:
    cernbox = os.environ['CERNBOX']

    Data.file_path_simple = f'{cernbox}/Run3/analysis_productions/for_local_tests/mc_turbo.root'
    Data.file_path_fail   = f'{cernbox}/tests/dmu/rfprinter/fail.root'
# -------------------------------------------------
def test_simple():
    '''
    Test basic printing
    '''

    obj = RFPrinter(path=Data.file_path_simple)
    obj.save(to_screen=True)
# -------------------------------------------------
def test_raise_on_fail():
    '''
    Test raise_on_fail flag 
    '''

    obj = RFPrinter(path=Data.file_path_fail)
    obj.save(raise_on_fail=False)
    with pytest.raises(OSError) as exc_info:
        obj.save(raise_on_fail= True)
        assert exc_info.value.startswith('Cannot open:')
