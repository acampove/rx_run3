'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error

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
# -------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:ds_getter', 10)
    LogStore.set_level('dmu:ml:cv_predict'     , 10)
# -------------------------------------------
def _is_signal(sample : str, trigger : str) -> bool:
    if not trigger.endswith('_MVA'):
        return False

    if sample not in ['Bu_Kee_eq_btosllball05_DPC', 'Bu_Kmumu_eq_btosllball05_DPC']:
        return False

    return True
# -------------------------------------------
def _get_signal_samples():
    l_sig = [ (sam, trig) for sam, trig in Data.l_mc_sample if _is_signal(sam, trig) ]

    return l_sig
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', Data.l_mc_sample)
def test_no_mva(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class without BDT added
    '''

    log.info(f'Running over: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove = ['q2', 'bdt'])
    if cfg is None:
        return

    obj = DsGetter(cfg=cfg)
    _   = obj.get_rdf()
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_signal_samples())
def test_cmb_mva_mc_signal(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class with combinatorial MVA added only on signal samples
    '''

    log.info(f'\nTesting with: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=['q2', 'bdt'])
    if cfg is None:
        return

    cfg['mva']         = {
            'cmb' : {
                'low'    : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/low',
                'central': f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/central',
                'high'   : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/high',
                }
            }

    obj = DsGetter(cfg=cfg)
    rdf = obj.get_rdf()

    file_dir  = '/tmp/rx_selection/ds_getter/mva_cmb'
    os.makedirs(file_dir, exist_ok=True)

    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_signal_samples())
def test_prc_mva_mc_signal(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class with combinatorial MVA added only on signal samples
    '''

    log.info(f'\nTesting with: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=['q2', 'bdt'])
    if cfg is None:
        return

    cfg['mva']         = {
            'prc' : {
                'low'    : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/low',
                'central': f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/central',
                'high'   : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/high',
                }
            }

    obj = DsGetter(cfg=cfg)
    rdf = obj.get_rdf()

    file_dir  = '/tmp/rx_selection/ds_getter/mva'
    os.makedirs(file_dir, exist_ok=True)

    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_signal_samples())
def test_mva_mc_signal(sample : str, trigger : str) -> None:
    '''
    Test of DsGetter class with combinatorial MVA added only on signal samples
    '''

    log.info(f'\nTesting with: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=['q2', 'bdt'])
    if cfg is None:
        return

    cfg['mva']         = {
            'cmb' : {
                'low'    : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/low',
                'central': f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/central',
                'high'   : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/cmb/high',
                },
            'prc' : {
                'low'    : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/low',
                'central': f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/central',
                'high'   : f'/publicfs/ucas/user/campoverde/Data/RK/MVA/run3/{Data.MVA_VERSION}/RK/prc/high',
                }
            }

    obj = DsGetter(cfg=cfg)
    rdf = obj.get_rdf()

    file_dir  = '/tmp/rx_selection/ds_getter/mva_both'
    os.makedirs(file_dir, exist_ok=True)

    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
