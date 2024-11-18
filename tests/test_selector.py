'''
Module with tests for selector class
'''
import dataclasses

from dmu.logging.log_store import LogStore
from ROOT                  import RDataFrame
from post_ap.selector      import selector

log = LogStore.add_logger('post_ap:test_selector')
# --------------------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    dt_path = '/home/acampove/data/aprod/downloads/flt_27_08_2024_dt_2024_turbo/00231366_00000001_1.ftuple.root'
    mc_path = '/home/acampove/data/aprod/downloads/flt_29_08_2024_mc_2024_turbo_comp/bukee/mc_bu_jpsik_ee_12153001_nu4p3_magdown_turbo_hlt1_2_tupling_00231483_00000002_1.tuple.root'
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('post_ap:selector'  , 10)
    LogStore.set_level('rx_scripts:atr_mgr:mgr', 30)
# --------------------------------------
def test_mc():
    '''
    Test selection in MC
    '''
    rdf = RDataFrame('Hlt2RD_BuToKpEE', Data.mc_path)
    obj = selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=True)
    rdf = obj.run()
# --------------------------------------
def test_dt():
    '''
    Test selection in data
    '''

    rdf = RDataFrame('Hlt2RD_BuToKpEE', Data.dt_path)

    obj = selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=False)
    rdf = obj.run()
# --------------------------------------
def test_cfl():
    '''
    Test retrieving multiple dataframes, one after each cut 
    '''

    rdf = RDataFrame('Hlt2RD_BuToKpEE', Data.mc_path)

    obj   = selector(rdf=rdf, cfg_nam='cuts_EE_2024', is_mc=True)
    d_rdf = obj.run(as_cutflow=True)

    for key, rdf in d_rdf.items():
        num = rdf.Count().GetValue()

        log.info(f'{key:<20}{num:<20}')
# --------------------------------------
