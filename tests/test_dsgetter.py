'''
Module with tests for DsGetter class
'''
# pylint: disable=import-error, no-name-in-module

import os
import pytest
from dmu.logging.log_store  import LogStore
from ROOT                   import RDataFrame

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
@pytest.mark.parametrize('sample, trigger', _get_samples(kind='signal'))
def test_cmb_mva(sample : str, trigger : str) -> None:
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

    _check_mva(rdf, ['mva_cmb'])
    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_samples(kind='signal'))
def test_prc_mva(sample : str, trigger : str) -> None:
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

    _check_mva(rdf, ['mva_prc'])
    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_samples(kind='signal'))
def test_mva_signal(sample : str, trigger : str) -> None:
    '''
    Test adding both MVAs on signal MC
    '''
    _both_mva(sample, trigger)
# -------------------------------------------
@pytest.mark.parametrize('sample, trigger', _get_samples(kind='data'))
def test_mva_data(sample : str, trigger : str) -> None:
    '''
    Test adding both MVAs on data
    '''
    _both_mva(sample, trigger)
# -------------------------------------------
def _both_mva(sample : str, trigger : str) -> None:
    '''
    Underlying test for both MVAs
    '''
    log.info(f'\nTesting with: {sample}/{trigger}')

    cfg = tst.get_dsg_config(sample, trigger, is_rk=True, remove=['q2', 'bdt'])
    if cfg is None:
        return

    if sample.startswith('DATA_'):
        cfg['max_files'] = 10

    cfg['mva']         = {
            'cmb' : {
                'low'    : f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/cmb/low',
                'central': f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/cmb/central',
                'high'   : f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/cmb/high',
                },
            'prc' : {
                'low'    : f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/prc/low',
                'central': f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/prc/central',
                'high'   : f'{Data.mva_dir}/run3/{Data.MVA_VERSION}/RK/prc/high',
                }
            }

    obj = DsGetter(cfg=cfg)
    rdf = obj.get_rdf()

    file_dir  = '/tmp/rx_selection/ds_getter/mva_both'
    os.makedirs(file_dir, exist_ok=True)

    _check_mva(rdf, ['mva_cmb', 'mva_prc'])

    file_path = f'{file_dir}/{sample}_{trigger}.root'
    rdf.Snapshot('tree', file_path)
# -------------------------------------------
def _check_mva(rdf : RDataFrame, l_col_needed : list[str]):
    l_col_found = [ name.c_str() for name in rdf.GetColumnNames() ]

    fail = False
    for col_needed in l_col_needed:
        if col_needed not in l_col_found:
            log.warning(f'Missing {col_needed}')

    if fail:
        raise ValueError('At least one column not found')
# -------------------------------------------
