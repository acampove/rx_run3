'''
Script with tests for PFNReader class
'''
from importlib.resources import files

import yaml
import pytest
import dmu.generic.utilities as gut

from post_ap.pfn_reader import PFNReader

# -----------------------------
class Data:
    '''
    Class used to store shared data
    '''
    gut.TIMER_ON=True
    l_arg_simple : list[tuple[str,str,int]] = [
            ('btoxll_mva_2024_nopid', 'simulation',   6),
            (           'rd_ap_2024',       'data',   8)]
# -----------------------------
def _get_cfg() -> dict:
    config_path = files('post_ap_data').joinpath('v1.yaml')
    config_path = str(config_path)

    with open(config_path, encoding='utf-8') as ifile:
        return yaml.safe_load(ifile)
# -----------------------------
@gut.timeit
@pytest.mark.parametrize('production, nickname, expected', Data.l_arg_simple)
def test_simple(production : str, nickname : str, expected : int):
    '''
    Test simple reading
    '''
    cfg    = _get_cfg()

    reader = PFNReader(cfg=cfg)
    d_pfn  = reader.get_pfns(production=production, nickname=nickname)

    assert len(d_pfn) == expected
