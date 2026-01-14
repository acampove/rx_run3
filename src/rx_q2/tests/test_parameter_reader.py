'''
This module contains functions to test the ParameterReader class
'''
import os
import pytest
import pandas as pnd

from pathlib     import Path
from rx_q2       import ParameterReader
from rx_q2       import ScalesConf
from dmu         import LogStore
from dmu.generic import utilities as gut

log=LogStore.add_logger('rx_q2:test_parameter_reader')
# ----------------------
@pytest.mark.parametrize('sample', ['rk_ee_2024', 'rk_mm_2024'])
def test_simple(sample : str) -> None:
    '''
    Simplest test of tool
    '''
    ANADIR = os.environ['ANADIR']
    path   = Path(ANADIR) / f'q2/fits/v6/dat/{sample}/0_1_nom/parameters.json'
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

    cfg    = ScalesConf(**data)
    rdr    = ParameterReader(cfg = cfg)
    l_data = [ rdr.read(path = path) for _ in range(3) ]

    df = pnd.DataFrame(l_data)

    print('')
    print(df)
    print(df.dtypes)
