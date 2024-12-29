'''
Module containing utilities for tests of kernel functions
'''
# pylint: disable=import-error, line-too-long

import os

from dataclasses             import dataclass
from ROOT                    import RDataFrame
from dmu.logging.log_store   import LogStore
from rx_kernel.config_holder import ConfigHolder

log=LogStore.add_logger('rx_common:test_utilities')
# --------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    nfiles   = 10
    nentries = 100

    d_tree_name = {True : 'DecayTree', False : 'DT'}
# -------------------------
def _get_conf(is_run3 : bool) -> dict[str,str]:
    cfg_run12 = {
            'project' : 'RK',
            'analysis': 'EE',
            'sample'  : 'Bd2K2EE',
            'q2bin'   : 'central',
            'year'    : '18',
            'polarity': 'MD',
            'trigger' : 'L0L',
            'hlt2'    : 'none',
            'data_dir': '/tmp/test_tuple_holder',
            'trg_cfg' : 'exclusive',
            'brem'    : '0G',
            'track'   : 'LL'}

    cfg_run3 = {
            'project'   : 'RK',
            'analysis'  : 'EE',
            'sample'    : 'data_24_magdown_24c4',
            'data_dir'  : '/tmp/test_tuple_holder',
            'hlt2'      : 'Hlt2RD_BuToKpEE_MVA',
            'tree_name' : 'DecayTree',
            'q2bin'     : 'central',
            'year'      : '24',
            'polarity'  : 'MD',
            'brem'      : '0G',
            'track'     : 'LL',
            'cut_opt'   : '',
            'wgt_opt'   : '',
            'tup_opt'   : '',
            }

    return cfg_run3 if is_run3 else cfg_run12
# -------------------------
def get_config_holder(is_run3 : bool):
    '''
    Function returns instance of C++ ConfigHolder.

    is_run3: It will return the object for Run3 configs if true, otherwise, something that works for Run1/2
    '''
    cfg = _get_conf(is_run3)
    obj = ConfigHolder(cfg, is_run3)

    return obj
# -------------------------
def make_inputs(is_run3 : bool) -> list[str]:
    '''
    Utility function taking configuration dictionary
    and making a set of ROOT files used for tests, the config looks like:

    'nfiles'  : 10,
    'nentries': 100,
    'data_dir': '/tmp/test_tuple_holder',
    'sample'  : 'data_24_magdown_24c4',
    'hlt2'    : 'Hlt2RD_BuToKpEE_MVA'

    Parameters
    ---------------
    is_run3: Bool specifying for what dataset to make test inputs

    Returns
    ---------------
    List of paths to files created
    '''

    cfg_inp   = _get_conf(is_run3)
    inp_dir   = f'{cfg_inp["data_dir"]}/{cfg_inp["sample"]}/{cfg_inp["hlt2"]}'
    tree_name = Data.d_tree_name[is_run3]

    log.info(f'Sending test inputs to: {inp_dir}')

    os.makedirs(inp_dir, exist_ok=True)

    l_file_path = []
    for i_file in range(Data.nfiles):
        file_path = _make_input(tree_name, inp_dir, i_file, Data.nentries)
        l_file_path.append(file_path)

    return l_file_path
# -------------------------
def _make_input(tree_name : str, inp_dir : str, i_file : int, nentries : int) -> str:
    rdf = RDataFrame(nentries)
    rdf = rdf.Define('a', '1')
    rdf = rdf.Define('b', '2')

    file_path = f'{inp_dir}/file_{i_file:03}.root'
    if os.path.isfile(file_path):
        return file_path

    rdf.Snapshot(tree_name, file_path)

    return file_path
# -------------------------
