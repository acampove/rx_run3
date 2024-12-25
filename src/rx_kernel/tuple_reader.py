'''
Module containing code to interface with TupleReader class
'''

from ROOT import TupleReader as TupleReader_cpp

def TupleReader(*args) -> TupleReader_cpp:
    if len(args) == 0:
        return TupleReader_cpp()

