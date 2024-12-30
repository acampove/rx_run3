'''
Module used to put RX core utilities, written in c++ in namespace
'''
# pylint: disable=import-error

from dmu.logging.log_store import LogStore
from rx_common             import utilities as ut


log=LogStore.add_logger('rx_common:kernel')
log.debug('Initializing kernel project')
ut.initialize_project('kernel')

# SettingDef cannot be accessed unti
from ROOT import SettingDef
from ROOT import MessageSvc

MessageSvc.Initialize(0)

config_dir = ut.get_config_dir()
SettingDef.AllowedConf.Initialize(config_dir)
