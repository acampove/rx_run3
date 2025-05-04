'''
Module with tests for ParameterLibrary class
'''

from dmu.stats.parameters import ParameterLibrary as PL

def test_print():
    '''
    Will test printing parameters
    '''
    PL.print_parameters()
