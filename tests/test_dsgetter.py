'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error

import pytest
from dmu.logging.log_store  import LogStore

import rx_selection.tests as tst
from rx_selection.ds_getter import DsGetter

log = LogStore.add_logger('rx_selection:test_dsgetter')
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', tst.get_mc_samples(is_rk=True, included=''))
def test_simple(sample : str, trigger : str) -> None:
    '''
    Simplest test of DsGetter class
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True)
    if cfg is None:
        return

    obj = DsGetter(cfg=cfg)
    _   = obj.get_rdf()
# -------------------------------------------
