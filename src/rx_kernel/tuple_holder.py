'''
Module containing python interface to C++ TupleHolder 
'''

from ROOT import TupleHolder as TupleHolder_cpp

def TupleHolder() -> TupleHolder_cpp:
    '''
    Function returning TupleHolder c++ implementation's instance
    '''
    obj = TupleHolder_cpp()

    return obj
