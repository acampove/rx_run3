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
            ('rd_ap_2024', 'test_w31_34_v1r2266_ee', 1),
            ('rd_ap_2024', 'test_w35_37_v1r2266_ee', 1),
            ('rd_ap_2024', 'test_w37_39_v1r2266_ee', 1),
            ('rd_ap_2024', 'test_w40_42_v1r2266_ee', 1),
            ('rd_ap_2024', 'test_w31_34_v1r2266_mm', 1),
            ('rd_ap_2024', 'test_w35_37_v1r2266_mm', 1),
            ('rd_ap_2024', 'test_w37_39_v1r2266_mm', 1),
            ('rd_ap_2024', 'test_w40_42_v1r2266_mm', 1),
            ('rd_ap_2024', 'test_w31_34_v1r2437_ee', 1),
            ('rd_ap_2024', 'test_w35_37_v1r2437_ee', 1),
            ('rd_ap_2024', 'test_w37_39_v1r2437_ee', 1),
            ('rd_ap_2024', 'test_w40_42_v1r2437_ee', 1),
            ('rd_ap_2024', 'test_w31_34_v1r2437_mm', 1),
            ('rd_ap_2024', 'test_w35_37_v1r2437_mm', 1),
            ('rd_ap_2024', 'test_w37_39_v1r2437_mm', 1),
            ('rd_ap_2024', 'test_w40_42_v1r2437_mm', 1)]
# -----------------------------
def _get_cfg() -> dict:
    config_path = files('post_ap_data').joinpath('post_ap/rx/v6.yaml')
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
    npfn   = len(d_pfn)

    assert npfn != 0
