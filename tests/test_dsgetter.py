'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error, no-name-in-module

import os
import pytest
from dmu.logging.log_store  import LogStore

import rx_selection.tests as tst
from rx_selection.ds_getter import DsGetter

log = LogStore.add_logger('rx_selection:test_dsgetter')
# -------------------------------------------
class Data:
    '''
    Class with shared attributes
    '''

    l_mc_sample = tst.get_mc_samples(is_rk=True)
    l_dt_sample = tst.get_dt_samples(is_rk=True)

    MVA_VERSION = 'v5'
    mva_dir     = os.environ['MVADIR']
# -------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:ds_getter', 10)
    LogStore.set_level('dmu:ml:cv_predict'     , 10)
# -------------------------------------------
def _is_kind(kind : str, sample : str, trigger : str) -> bool:
    if not trigger.endswith('_MVA'):
        return False

    if 'Bu' not in trigger:
        return False

    if kind == 'data'   and sample.startswith('DATA_'):
        return True

    if kind == 'mc'     and not sample.startswith('DATA_'):
        return True

    if kind == 'signal' and sample in ['Bu_Kee_eq_btosllball05_DPC', 'Bu_Kmumu_eq_btosllball05_DPC']:
        return True

    return False
# -------------------------------------------
def _get_samples(kind : str) -> list[tuple[str,str]]:
    l_sig = [ (sam, trig) for sam, trig in Data.l_mc_sample + Data.l_dt_sample if _is_kind(kind, sam, trig) ]

    return l_sig
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_samples(kind='mc'))
def test_select(sample : str, trigger : str) -> None:
    '''
    Tests selection
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove = ['q2', 'bdt'])
    if cfg is None:
        return

    obj = DsGetter(cfg=cfg)
    _   = obj.get_rdf()
# -------------------------------------------
