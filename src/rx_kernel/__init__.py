'''
Module used to put RX core utilities, written in c++ in namespace
'''
# pylint: disable=import-error

from dmu.logging.log_store import LogStore

from rx_common             import utilities as ut

log=LogStore.add_logger('rx_common:kernel')

LIB_PATH = ut.get_lib_path('kernel')
ut.load_library(LIB_PATH)
ut.include_headers()
