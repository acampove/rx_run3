'''
This module is needed to control how pytest will run tests
'''

import pytest
from dmu.generic        import utilities as gut
from rx_data.rdf_getter import RDFGetter

# -----------------------------------------------
@pytest.fixture(autouse=True)
def rdf_getter_configuration():
    '''
    This will configure RDFGetter for tests
    '''
    with RDFGetter.max_entries(value=1000):
        yield

    gut.TIMER_ON = True
# -----------------------------------------------
