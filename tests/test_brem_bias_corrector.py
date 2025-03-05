'''
Module with functions needed to test BremBiasCorrector class 
'''

from vector                      import MomentumObject4D as v4d
from rx_data.brem_bias_corrector import BremBiasCorrector

# -----------------------------------------------
def _get_input():
    brem = v4d(pt=5_000, eta=3.0, phi=1.0, mass=0.511)

    return brem, 10, 10
# -----------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    brem, row, col = _get_input()

    obj = BremBiasCorrector()
    brem= obj.correct(brem=brem, row=row, col=col)

