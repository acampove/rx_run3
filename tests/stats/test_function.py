'''
Module with tests for Function class
'''

from dmu.stats.function import Function

def test_simple():
    '''
    Will test creation of function
    '''

    x = [0, 1, 2]
    y = [0, 1, 2]

    fun = Function(x=x, y=y)
