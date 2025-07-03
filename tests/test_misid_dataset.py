'''
Module meant to test MisIDDataset class
'''

import pytest

from dmu.logging.log_store  import LogStore
from rx_misid.misid_dataset import MisIDDataset

log = LogStore.add_logger('rx_misid:test_misid_dataset')
# -----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:misid_calculator' , 10)
    LogStore.set_level('rx_misid:misid_dataset'    , 10)
# -----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_with_leakage(q2bin : str):
    '''
    Returns all components, data and MC
    '''
    dst = MisIDDataset(q2bin=q2bin)
    d_df= dst.get_data(only_data=False)

    assert len(d_df) == 3
    assert 'data'    in d_df
    assert 'signal'  in d_df
    assert 'leakage' in d_df

    _plot_data(
            d_df =d_df,
            q2bin=q2bin,
            name ='with_leakage')
# -----------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_only_data(q2bin : str):
    '''
    Simplest test
    '''
    dst = MisIDDataset(q2bin=q2bin)
    d_df= dst.get_data(only_data=True)

    assert len(d_df) == 1
    assert 'data'    in d_df

    _plot_data(
            d_df =d_df,
            q2bin=q2bin,
            name ='no_leakage')
# -----------------------------------------------
