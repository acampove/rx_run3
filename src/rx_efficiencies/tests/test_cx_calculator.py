'''
This module is used to test the CXCalculator class
'''

import pytest

from rx_efficiencies import CXCalculator
from rx_common       import Project

# ----------------------
@pytest.mark.parametrize('project', [Project.rk, Project.rkst])
def test_cxcalculator(project : Project) -> None:
    '''
    Tests calculation for different projects
    '''
    obj      = CXCalculator(project = project)
    val, err = obj.calculate()

    assert val > 0
    assert err > 0

