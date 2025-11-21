import os
import pytest
from pathlib import Path

from fitter    import ParameterReader
from rx_common import Project
from rx_common import Trigger 
from rx_common import Qsq 
from dmu       import LogStore

log=LogStore.add_logger('fitter:test_parameter_reader')
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize() -> None:
    LogStore.set_level('fitter:parameter_reader', 10)
# -----------------------
def test_simple():
    '''
    Test simplest use of reader
    '''
    rdr = ParameterReader(name = 'mid_window')
    ms_sim = rdr(
        brem     = 1, 
        block    = 3, 
        trigger  = Trigger.rk_ee_os, 
        project  = Project.rk,
        q2bin    = Qsq.jpsi)

    print(ms_sim)
# -----------------------
def test_pars_path():
    '''
    Test context manager user to set path to parameters 
    '''
    ana_dir = Path(os.environ['ANADIR'])
    pars_path = ana_dir / 'fits/data/mid_window/parameters.parquet'

    with ParameterReader.inputs_from(pars_path = pars_path):
        rdr = ParameterReader(name = 'mid_window')
        ms_sim = rdr(
            brem     = 1, 
            block    = 3, 
            trigger  = Trigger.rk_ee_os, 
            project  = Project.rk,
            q2bin    = Qsq.jpsi)

        print(ms_sim)

