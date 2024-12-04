'''
Module with tests for selector class
'''
import os
from dataclasses         import dataclass
from importlib.resources import files

import pytest
from dmu.logging.log_store import LogStore
from ROOT                  import RDataFrame
from post_ap.selector      import Selector

log = LogStore.add_logger('post_ap:test_selector')
# --------------------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    mc_path : str
    dt_path : str
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    config_path               = files('post_ap_data').joinpath('v1.yaml')
    os.environ['CONFIG_PATH'] = str(config_path)

    LogStore.set_level('post_ap:selector'      , 10)
    LogStore.set_level('dmu:rdataframe:atr_mgr', 30)

    cern_box     = os.environ['CERNBOX']
    Data.mc_path = f'{cern_box}/Run3/analysis_productions/MC/local_tests/mc_2024_w31_34_magup_nu6p3_sim10d_pythia8_12143010_bu_jpsipi_mm_tuple.root'
    Data.dt_path = f'{cern_box}/Run3/analysis_productions/MC/local_tests/mc_2024_w31_34_magup_nu6p3_sim10d_pythia8_12143010_bu_jpsipi_mm_tuple.root'
# --------------------------------------
def _rename_branches(rdf : RDataFrame) -> RDataFrame:
    rdf = rdf.Define('B_const_mass_M', 'B_DTF_Jpsi_MASS')

    return rdf
# --------------------------------------
def test_mc():
    '''
    Test selection in MC
    '''

    rdf = RDataFrame('Hlt2RD_B0ToKpKmMuMu/DecayTree', Data.mc_path)
    rdf = _rename_branches(rdf)

    obj = Selector(rdf=rdf, is_mc=True)
    rdf = obj.run(sel_kind = 'bukmm')
# --------------------------------------
def test_dt():
    '''
    Test selection in data
    '''

    rdf = RDataFrame('Hlt2RD_B0ToKpKmMuMu/DecayTree', Data.dt_path)
    rdf = _rename_branches(rdf)

    obj = Selector(rdf=rdf, is_mc=False)
    rdf = obj.run(sel_kind = 'bukmm')
# --------------------------------------
def test_cfl():
    '''
    Test retrieving multiple dataframes, one after each cut 
    '''

    rdf          = RDataFrame('Hlt2RD_B0ToKpKmMuMu/DecayTree', Data.mc_path)
    rdf          = _rename_branches(rdf)

    obj   = Selector(rdf=rdf, is_mc=True)
    d_rdf = obj.run(sel_kind = 'bukmm', as_cutflow=True)

    for key, rdf in d_rdf.items():
        num = rdf.Count().GetValue()

        log.info(f'{key:<20}{num:<20}')
# --------------------------------------
