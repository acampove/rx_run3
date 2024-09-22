'''
Module holding unit tests for text transforming tools
'''

from importlib.resources   import files
from dataclasses           import dataclass

import pytest

from dmu.text.transformer  import transformer as txt_trf
from dmu.logging.log_store import LogStore


log=LogStore.add_logger('dmu:tests:test_transformer')
# -------------------------------------------------------------
@dataclass
class Data:
    '''
    Class used to store shared variables
    '''
    cfg : str
    txt : str
    out : str
@pytest.fixture(scope='module', autouse=True)
# -------------------------------------------------------------
def initialize():
    '''
    Will initialize variables, etc
    '''
    log.setLevel(10)

    log.info('Loading inputs')

    Data.txt = files('dmu_data').joinpath('text/transform.txt')
    Data.cfg = files('dmu_data').joinpath('text/transform.toml')

    LogStore.set_level('dmu:text:transformer', 10)
# -------------------------------------------------------------
def test_with_path_ext():
    '''
    Testing save_as
    '''
    Data.out = '/tmp/dmu_test/with_path_ext.txt'

    trf=txt_trf(txt_path=Data.txt, cfg_path=Data.cfg)
    trf.save_as(out_path=Data.out)
# -------------------------------------------------------------
def test_with_path():
    Data.out = '/tmp/dmu_test/with_path'

    trf=txt_trf(txt_path=Data.txt, cfg_path=Data.cfg)
    trf.save_as(out_path=Data.out)
# -------------------------------------------------------------
def test_settings():
    Data.out = '/tmp/dmu_test/settings'
    cfg      = files('dmu_data').joinpath('text/transform_set.toml')
    txt      = files('dmu_data').joinpath('text/transform_set.txt')

    trf=txt_trf(txt_path=txt, cfg_path=cfg)
    trf.save_as(out_path=Data.out)
# -------------------------------------------------------------
def main():
    initialize()

    test_settings()
    test_with_path()
    test_with_path_ext()
# -------------------------------------------------------------
if __name__ == '__main__':
    main()
