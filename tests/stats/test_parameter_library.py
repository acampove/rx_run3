'''
Module with tests for ParameterLibrary class
'''

import pytest
from dmu.stats.parameters import ParameterLibrary as PL

# ------------------------------------
@pytest.mark.parametrize('kind', ['gauss', 'cbl', 'cbr', 'suj'])
def test_print(kind : str):
    '''
    Will test printing parameters
    '''
    PL.print_parameters(kind=kind)
# ------------------------------------
