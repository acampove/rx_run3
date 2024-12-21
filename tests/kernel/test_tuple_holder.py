'''
Module containing tests for TupleHolder
'''

from rx_kernel.tuple_holder import TupleHolder

def test_default():
    '''
    Test for default constructor
    '''
    obj = TupleHolder()
    obj.PrintInline() 
