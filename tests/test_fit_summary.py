'''
Module with tests for FitSummary class 
'''

from fitter import FitSummary

# ----------------------
def test_simple() -> None:
    '''
    Simplest test
    '''
    smr = FitSummary(name='mid_window')
    smr.save()
    
