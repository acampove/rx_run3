'''
This module is needed to control how pytest will run tests
'''

import pytest
from rx_data.rdf_getter import RDFGetter

# -----------------------------------------------
@pytest.fixture(autouse=True)
def rdf_getter_configuration():
    '''
    This will configure RDFGetter for tests
    '''
    with RDFGetter.max_entries(value=1000):
        yield
# -----------------------------------------------
