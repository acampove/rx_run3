'''
Module with functions meant to test SampleSplitter class
'''
from importlib.resources import files

import yaml
import pytest
from dmu.stats.wdata       import Wdata
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
def _get_config() -> dict:
    cfg_path = files('rx_misid_data').joinpath('config.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg['splitting']
# -------------------------------------------------------
def _get_rdf():
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Range(10_000)

    return rdf
# -------------------------------------------------------
def _check_samples(samples : dict[str, Wdata]):
    fail = False
    for name, sample in samples.items():
        if sample.size == 0:
            log.warning(f'Empty sample: {name}')
            fail=True
            continue

        log.info(f'Sample: {name}')
        log.info(sample)

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
    d_sam = spl.get_samples()

    _check_samples(samples=d_sam)
# -------------------------------------------------------
