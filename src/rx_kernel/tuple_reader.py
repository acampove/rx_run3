'''
Module containing code to interface with TupleReader class
'''

from ROOT import TString
from ROOT import TupleReader as TupleReader_cpp

def TupleReader(*args) -> TupleReader_cpp:
    if len(args) == 0:
        return TupleReader_cpp()

    if len(args) == 1:
        tree_name = TString(args[0])

        return TupleReader_cpp(tree_name)

    if len(args) == 2:
        tree_name = TString(args[0])
        file_name = TString(args[1])

        return TupleReader_cpp(tree_name, file_name)

    narg = len(args)
    raise ValueError(f'Found invalid number of arguments: {narg}')
