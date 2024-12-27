'''
Module containing python interface to C++ TupleHolder
'''
# pylint: disable=invalid-name

from ROOT   import TString
from ROOT   import TupleHolder  as TupleHolder_cpp

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_common:tuple_holder')
# -----------------------------------------------------------
def TupleHolder(*args) -> TupleHolder_cpp:
    '''
    Function returning TupleHolder c++ implementation's instance
    '''

    if len(args) == 0:
        log.debug('Using default constructor')
        return TupleHolder_cpp()

    if len(args) == 2:
        log.debug('Using constructor with ConfigHolder and string option')
        [cfg, opt] = args
        opt = TString(opt)

        return TupleHolder_cpp(cfg, opt)

    if len(args) == 4:
        log.debug('Using constructor with tree name and file path')
        [cfg, file_path, tree_path, opt] = args

        file_path = TString(file_path)
        tree_path = TString(tree_path)
        opt       = TString(opt)

        return TupleHolder_cpp(cfg, file_path, tree_path, opt)

    raise ValueError('Invalid number of arguments, allowed: 0, 2, 4')
# -----------------------------------------------------------
