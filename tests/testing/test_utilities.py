'''
Module with unit tests for testing/utilities.py functions
'''

from ROOT import RDataFrame

import dmu.testing.utilities as ut

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:tests:test_utilities')
# ----------------------------------------------
def _check_rdf(rdf):
    if not isinstance(rdf, RDataFrame):
        kind = str(type(rdf))
        log.error(f'Object is not a dataframe but: {kind}')
        raise ValueError
# ----------------------------------------------
def test_get_rdf():
    '''
    Test for toy dataframe getter
    '''
    rdf = ut.get_rdf(kind='sig')
    _check_rdf(rdf)

    rdf = ut.get_rdf(kind='bkg')
    _check_rdf(rdf)
# ----------------------------------------------
def main():
    '''
    Tests start here
    '''
    test_get_rdf()
# ----------------------------------------------
if __name__ == '__main__':
    main()
