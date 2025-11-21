'''
Script with tests for PFNReader class
'''
import pytest
import dmu.generic.utilities as gut

from post_ap.pfn_reader import PFNReader

# -----------------------------
class Data:
    '''
    Class used to store shared data
    '''
    l_arg_nopid : list[tuple[str,int]] = [
        ('w31_34_v1r3411', 1),
        ('w35_37_v1r3411', 1),
        ('w37_39_v1r3411', 1),
        ('w40_42_v1r3411', 2)]
# -----------------------------
@pytest.mark.parametrize('nickname, expected', Data.l_arg_nopid)
def test_simple(nickname : str, expected : int):
    '''
    Test simple reading
    '''
    production = 'btoxll_mva_2024_nopid'

    cfg    = gut.load_conf(package='post_ap_data', fpath='tests/pfn_reader/simple.yaml')

    reader = PFNReader(cfg=cfg)
    d_pfn  = reader.get_pfns(production=production, nickname=nickname)
    npfn   = len(d_pfn)

    assert npfn == expected
