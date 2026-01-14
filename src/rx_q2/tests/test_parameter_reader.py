'''
This module contains functions to test the ParameterReader class
'''
import os
import pandas as pnd

from pathlib     import Path
from rx_q2       import ParameterReader
from rx_q2       import ScalesConf
from dmu         import LogStore
from dmu.generic import utilities as gut

log=LogStore.add_logger('rx_q2:test_parameter_reader')
# ----------------------
def test_simple() -> None:
    '''
    Simplest test of tool
    '''
    ANADIR = os.environ['ANADIR']
    path   = Path(ANADIR) / 'q2/fits/v6/dat/rk_ee_2024/1_1_nom/parameters.json'
    if not path.exists():
        raise ValueError(f'Cannot find: {path}')

    data            = gut.load_data(package='rx_q2_data', fpath='plots/scales.yaml')
    data['kind']    = 'q2'
    data['regex']   = r'(\d)_(\d)_nom'

    data['vers']    = 'dummy'
    data['year']    = 'dummy'
    data['proj']    = 'dummy'
    data['inp_dir'] = Path('dummy')
    data['out_dir'] = Path('dummy')

    cfg  = ScalesConf(**data)
    rdr  = ParameterReader(cfg = cfg)
    srs  = rdr.read(path = path)

    assert isinstance(srs, pnd.Series)
    
    print(srs)
