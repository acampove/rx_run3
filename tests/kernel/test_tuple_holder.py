'''
Module containing tests for TupleHolder
'''
# pylint: disable=import-error, wrong-import-order

from dataclasses import dataclass

import pytest
from rx_common               import utilities as ut
from rx_kernel.tuple_holder  import TupleHolder
from rx_kernel.config_holder import ConfigHolder
from rx_kernel               import allowed_conf

from ROOT import MessageSvc
from ROOT import ConfigHolder as ConfigHolder_cpp

from dmu.logging.log_store import LogStore

MessageSvc.Initialize(-1)

log=LogStore.add_logger('rx_common:test_tuple_holder')
# -------------------------
@dataclass
class Data:
    '''
    Class used to share data between tests
    '''

    l_tuple_opt = [
            'gng',
            'pro',
            'cre',
            'spl',
            'rap',
            'tmp',
            'chainexctrg',
            'pap',
            'ap'
            ]
# -----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    allowed_conf.Initialize('/home/acampove/Tests/rx_samples')

    cfg_inp  = {
            'nfiles'  : 10,
            'nentries': 100,
            'data_dir': '/tmp/test_tuple_holder',
            'sample'  : 'data_24_magdown_24c4',
            'hlt2'    : 'Hlt2RD_BuToKpEE_MVA'}

    ut.make_inputs(cfg_inp)

    LogStore.set_level('rx_common:config_holder', 10)
    LogStore.set_level('rx_common:tuple_holder' , 10)
# -------------------------
def _get_config_holder(is_run3 : bool) -> ConfigHolder_cpp:
    cfg_run12 = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'Bd2KstPsiEE',
            'q2bin'   : 'central',
            'year'    : '18',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL'}

    cfg_run3 = {
            'project'   : 'RK',
            'analysis'  : 'EE',
            'data_dir'  : Data.data_dir, 
            'sample'    : Data.sample,
            'hlt2'      : Data.hlt2, 
            'tree_name' : 'DecayTree',
            'trigger'   : '',
            'q2bin'     : 'central',
            'year'      : '24',
            'polarity'  : 'MD',
            'brem'      : '0G',
            'track'     : 'LL'}

    cfg = cfg_run3 if is_run3 else cfg_run12

    return ConfigHolder(cfg=cfg)
# -------------------------
def test_default():
    '''
    Test for default constructor
    '''
    obj = TupleHolder()
    obj.PrintInline()
# -------------------------
@pytest.mark.parametrize('option' , Data.l_tuple_opt)
@pytest.mark.parametrize('is_run3', [True, False])
def test_options(option : str, is_run3 : bool):
    '''
    Will test options
    '''
    ch  = _get_config_holder(is_run3)
    obj = TupleHolder(ch, option)
    obj.PrintInline()
# -------------------------
@pytest.mark.parametrize('is_run3', [True, False])
def test_arg(is_run3 : bool):
    '''
    Test for constructor with file and tuple args
    '''
    file_path = '/home/acampove/cernbox/Run3/filtering/data/dec_07_2024_data/data_24_magdown_turbo_24c1_Hlt2RD_B0ToKpPimEE_0062a7d56d.root'
    tree_path = 'KEE'

    ch  = _get_config_holder(is_run3)
    obj = TupleHolder(ch, file_path, tree_path, 'pap')
    obj.PrintInline()

    rdr = obj.GetTupleReader()
    rdr.PrintInline()
# -------------------------
@pytest.mark.parametrize('is_run3', [True])
def test_postap(is_run3 : bool):
    '''
    Test for constructor for post_ap ntuples 
    '''

    ch  = _get_config_holder(is_run3)
    obj = TupleHolder(ch, 'pap')
    #trd = obj.GetTupleReader()
    #tup = trd.Tuple()
    #tup.Print()
