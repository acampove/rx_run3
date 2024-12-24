'''
Module containing tests for TupleHolder
'''
# pylint: disable=import-error, wrong-import-order

from dataclasses import dataclass

import pytest
from rx_kernel.tuple_holder  import TupleHolder
from rx_kernel.config_holder import ConfigHolder

from ROOT import MessageSvc
from ROOT import ConfigHolder as ConfigHolder_cpp


MessageSvc.Initialize(-1)
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
# -------------------------
def _get_config_holder(is_run3 : bool) -> ConfigHolder_cpp:
    cfg_run12 = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'LPT',
            'q2bin'   : 'central',
            'year'    : '18',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL'}

    cfg_run3 = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'data_24_magdown_24c4',
            'q2bin'   : 'central',
            'year'    : '18',
            'polarity': 'MD',
            'brem'    : '0G',
            'track'   : 'LL'}

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
@pytest.mark.parametrize('option', Data.l_tuple_opt)
def test_options(option : str):
    '''
    Will test options
    '''
    ch  = _get_config_holder()
    obj = TupleHolder(ch, option)
    obj.PrintInline()
# -------------------------
def test_arg():
    '''
    Test for constructor with file and tuple args
    '''
    file_path = '/home/acampove/cernbox/Run3/analysis_productions/for_local_tests/mc_turbo.root'
    tree_path = 'DecayTree'

    ch  = _get_config_holder()
    obj = TupleHolder(ch, file_path, tree_path, '')
    obj.PrintInline()
# -------------------------
