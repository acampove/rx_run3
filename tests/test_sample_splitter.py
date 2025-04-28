'''
Module with functions meant to test SampleSplitter class
'''
from importlib.resources import files

import yaml
import pytest
import pandas as pnd
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_misid.splitter     import SampleSplitter

log=LogStore.add_logger('rx_misid:test_splitter')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    l_hadron_id = ['kaon', 'pion']
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:splitter', 10)
# -------------------------------------------------------
def _get_config() -> dict:
    cfg_path = files('rx_misid_data').joinpath('config.yaml')
    log.info(f'Picking up config from: {cfg_path}')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg['splitting']
# -------------------------------------------------------
def _get_rdf():
    gtr = RDFGetter(sample='DATA_24_MagUp_24c1', trigger='Hlt2RD_BuToKpEE_MVA_ext')
    rdf = gtr.get_rdf()

    return rdf
# -------------------------------------------------------
def _check_stats(df : pnd.DataFrame):
    fail = False
    log.info(40 * '-')
    log.info(f'{"Kind":<20}{"Entries":<20}')
    log.info(40 * '-')
    for kind, df_kind in df.groupby('kind'):
        if len(df_kind) == 0:
            log.warning(f'Empty sample: {kind}')
            fail=True
            continue

        nentries = len(df_kind)

        log.info(f'{kind:<20}{nentries:<20}')
    log.info(40 * '-')

    assert not fail
# -------------------------------------------------------
@pytest.mark.parametrize('hadron_id', Data.l_hadron_id)
@pytest.mark.parametrize('is_bplus' ,    [True, False])
def test_simple(hadron_id : str, is_bplus : bool):
    '''
    Tests simplest splitting
    '''
    rdf   = _get_rdf()
    cfg   = _get_config()

    spl   = SampleSplitter(rdf=rdf, hadron_id=hadron_id, is_bplus=is_bplus, cfg=cfg)
    df    = spl.get_samples()

    _check_stats(df=df)
# -------------------------------------------------------
