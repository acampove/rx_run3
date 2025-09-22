'''
Script with functions needed to test functions in selection.py
'''
import os
import pytest
from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
class Data:
    '''
    data class
    '''
    DATADIR = os.environ['ANADIR'] + '/Data'
    l_q2bin = ['low', 'cen_low', 'central', 'cen_high', 'psi2', 'high']

    out_dir = '/tmp/tests/rx_selection/selection'
# --------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_selection:selection'     , 10)
    LogStore.set_level('rx_selection:test_selection', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_BuToKpMuMu_MVA'])
@pytest.mark.parametrize('q2bin'  , Data.l_q2bin)
def test_read_selection(trigger : str, q2bin : str):
    '''
    Test reading the selection
    '''
    d_sel = sel.selection(trigger=trigger, q2bin=q2bin, process='DATA')
    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('q2bin' , Data.l_q2bin)
def test_custom_selection(sample : str, q2bin : str):
    '''
    This function tests the custom_selection
    context manager
    '''
    with sel.custom_selection(d_sel={'cut' : 'val'}):
        d_cut = sel.selection(process=sample, q2bin=q2bin, trigger='Hlt2RD_BuToKpEE_MVA')

    assert 'cut' in d_cut
    assert d_cut['cut'] == 'val'
# --------------------------
def test_multiple_custom_selection():
    '''
    Tests that we cannot call custom_selection manager in a nested way
    '''
    sample = 'Bu_Kee_eq_btosllball05_DPC'
    q2bin  = 'central'
    trigger= 'Hlt2RD_BuToKpEE_MVA'

    with pytest.raises(sel.MultipleSelectionOverriding):
        with sel.custom_selection(d_sel={'cut' : 'val'}):
            with sel.custom_selection(d_sel={'cut' : 'vol'}):
                sel.selection(process=sample, q2bin=q2bin, trigger=trigger)
# --------------------------
def test_multiple_custom_selection_override():
    '''
    Tests calling custom_selection with overriding flag
    '''
    sample = 'Bu_Kee_eq_btosllball05_DPC'
    q2bin  = 'central'
    trigger= 'Hlt2RD_BuToKpEE_MVA'

    with sel.custom_selection(d_sel={'cut' : 'val'}, force_override=True):
        d_cut_1 = sel.selection(process=sample, q2bin=q2bin, trigger=trigger)
        assert d_cut_1['cut'] == 'val'
        with sel.custom_selection(d_sel={'cut' : 'vol'}, force_override=True):
            d_cut_2 = sel.selection(process=sample, q2bin=q2bin, trigger=trigger)
            assert d_cut_2['cut'] == 'vol'

        d_cut_1 = sel.selection(process=sample, q2bin=q2bin, trigger=trigger)
        assert d_cut_1['cut'] == 'val'

    d_cut = sel.selection(process=sample, q2bin=q2bin, trigger=trigger)
    assert 'cut' not in d_cut
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('block' , [1, 2, 3, 4, 5, 6, 7, 8])
def test_block_overriding(sample : str, block : int):
    '''
    Tests overriding of block cut for simulation
    '''
    old_cut_block = f'block == {block}'
    rep_cut_block =  'block == 2'
    with sel.custom_selection(d_sel={'block' : old_cut_block}):
        d_sel = sel.selection(q2bin='central', process=sample, trigger='Hlt2RD_BuToKpEE_MVA')

    new_cut_block = d_sel['block']
    if sample == 'DATA_24_MagDown_24c2':
        assert old_cut_block == new_cut_block
        return

    if block in [3, 4]:
        assert new_cut_block == rep_cut_block
    else:
        assert old_cut_block == new_cut_block
# --------------------------
def test_custom_selection_nested():
    '''
    Check that nesting of context managers does not break upstream (original) custom selection
    '''
    csel_000 = sel.Data.d_custom_selection
    with sel.custom_selection(d_sel={'cut_1' : 'val_1'}):
        csel_001 = sel.Data.d_custom_selection
        with sel.custom_selection(d_sel={'cut_2' : 'val_2'}, force_override=True):
            csel_002 = sel.Data.d_custom_selection
            with sel.custom_selection(d_sel={'cut_3' : 'val_3'}, force_override=True):
                csel_003 = sel.Data.d_custom_selection

            csel_012  = sel.Data.d_custom_selection
        csel_011  = sel.Data.d_custom_selection
    csel_010  = sel.Data.d_custom_selection

    assert csel_000 == csel_010
    assert csel_001 == csel_011
    assert csel_002 == csel_012
    assert csel_003 == {'cut_3' : 'val_3'}
# --------------------------
def test_update_selection():
    '''
    Tests that manger can add cuts to existing selection
    '''
    isel_001 = {'cut_1' : 'val_1'}
    with sel.update_selection(d_sel=isel_001):
        osel_001 = sel.Data.d_custom_selection

    assert    isel_001  ==    osel_001
    assert id(isel_001) != id(osel_001)

    csel_001 = {'cut_1' : 'val_1'}
    isel_001 = {'cut_2' : 'val_2'}
    with sel.custom_selection(d_sel=csel_001):
        with sel.update_selection(d_sel=isel_001):
            osel_001 = sel.Data.d_custom_selection

    csel_001.update(isel_001)

    assert    csel_001  == osel_001
    assert id(isel_001) != id(osel_001)

    isel_001 = {'cut_1' : 'val_1'}
    isel_002 = {'cut_2' : 'val_2'}
    with sel.update_selection(d_sel=isel_001):
        with sel.update_selection(d_sel=isel_002):
            osel_002 = sel.Data.d_custom_selection

    isel_001.update(isel_002)

    assert    isel_001  ==    osel_002
    assert id(isel_001) != id(osel_002)
# --------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_no_truth(trigger : str): 
    '''
    Test reading the selection
    '''
    sample= 'Bd_Kstee_eq_btosllball05_DPC'
    d_sel = sel.selection(
        trigger   =trigger, 
        q2bin     ='central', 
        skip_truth=True,
        process   =sample)

    assert d_sel['truth'] == '(1)'
