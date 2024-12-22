'''
Module containing python interface to C++ TupleHolder
'''
from ROOT   import TString
from ROOT   import TupleHolder  as TupleHolder_cpp

# pylint: disable=invalid-name
# -----------------------------------------------------------
def TupleHolder(*args) -> TupleHolder_cpp:
    '''
    Function returning TupleHolder c++ implementation's instance
    '''

    if len(args) == 0:
        return TupleHolder_cpp()

    if len(args) == 2:
        [cfg, opt] = args
        opt = TString(opt)

        return TupleHolder_cpp(cfg, opt)

    if len(args) == 4:
        [cfg, file_path, tree_path, opt] = args

        file_path = TString(file_path)
        tree_path = TString(tree_path)
        opt       = TString(opt)

        return TupleHolder_cpp(cfg, file_path, tree_path, opt)

    raise ValueError('Invalid number of arguments, allowed: 0, 2, 4')
# -----------------------------------------------------------
