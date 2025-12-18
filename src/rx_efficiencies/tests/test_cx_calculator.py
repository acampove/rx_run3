'''
This module is used to test the CXCalculator class
'''

import pytest

from rx_efficiencies import CXCalculator
from rx_common       import Project, Qsq
from dmu.generic     import utilities as gut

# ----------------------
@pytest.mark.parametrize('project', [Project.rk, Project.rkst])
@pytest.mark.parametrize('qsq'    , [Qsq.low, Qsq.central, Qsq.high])
def test_cxcalculator(project : Project, qsq : Qsq) -> None:
    '''
    Tests calculation for different projects
    '''
    with gut.environment({'RXSEED' : '123'}):
        obj      = CXCalculator(project = project, qsq = qsq)
        val, err = obj.calculate()

    assert val > 0
    assert err > 0

