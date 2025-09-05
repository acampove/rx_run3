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
    gut.TIMER_ON=True
    l_arg_rd_ap_2024 : list[tuple[str,str,int]] = [
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

    l_arg_nopid      : list[tuple[str,str,int]] = [
        ('btoxll_mva_2024_nopid', 'w31_34_v1r3399', 1),
        ('btoxll_mva_2024_nopid', 'w35_37_v1r3399', 1),
        ('btoxll_mva_2024_nopid', 'w37_39_v1r3399', 1),
        ('btoxll_mva_2024_nopid', 'w40_42_v1r3399', 2),
        ('btoxll_mva_2024_nopid', 'w31_34_v1r3411', 1),
        ('btoxll_mva_2024_nopid', 'w35_37_v1r3411', 1),
        ('btoxll_mva_2024_nopid', 'w37_39_v1r3411', 1),
        ('btoxll_mva_2024_nopid', 'w40_42_v1r3411', 2)]
# -----------------------------
@gut.timeit
@pytest.mark.parametrize('production, nickname, expected', Data.l_arg_nopid)
def test_simple(production : str, nickname : str, expected : int):
    '''
    Test simple reading
    '''
    cfg    = gut.load_data(package='post_ap_data', fpath='post_ap/nopid/v1.yaml')

    reader = PFNReader(cfg=cfg)
    d_pfn  = reader.get_pfns(production=production, nickname=nickname)
    npfn   = len(d_pfn)

    assert npfn == expected
