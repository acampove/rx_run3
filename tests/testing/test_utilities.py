'''
Module with unit tests for testing/utilities.py functions
'''
# pylint: disable=no-name-in-module

import pytest
from ROOT import RDataFrame

import dmu.testing.utilities as ut

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:tests:test_utilities')
# ----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:testing:utilities', 10)
# ----------------------------------------------
def _check_rdf(rdf : RDataFrame) -> None:
    if not isinstance(rdf, RDataFrame):
        kind = str(type(rdf))
        log.error(f'Object is not a dataframe but: {kind}')
        raise ValueError

    log.info('')
    rdf.Display(nRows=20).Print()
# ----------------------------------------------
def test_get_rdf():
    '''
    Test for toy dataframe getter
    '''
    rdf = ut.get_rdf(kind='sig', nentries=10)
    _check_rdf(rdf)

    rdf = ut.get_rdf(kind='bkg', nentries=10)
    _check_rdf(rdf)
# ----------------------------------------------
def test_get_rdf_with_nans():
    '''
    Test for toy dataframe getter
    '''
    rdf = ut.get_rdf(kind='sig', nentries=10, add_nans=True)
    _check_rdf(rdf)

    rdf = ut.get_rdf(kind='bkg', nentries=10, add_nans=True)
    _check_rdf(rdf)
# ----------------------------------------------
