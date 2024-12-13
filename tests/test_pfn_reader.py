'''
Script with tests for PFNReader class
'''
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
            ('rd_ap_2024', 'simulation_turbo',   75),
            ('rd_ap_2024',       'data_turbo',    8)]
# -----------------------------
def _get_cfg() -> dict:
    config_path = '/home/acampove/Packages/config_files/post_ap/v3.yaml' 
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

    assert npfn == expected
