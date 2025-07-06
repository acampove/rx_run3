'''
Script with functions needed to test functions in selection.py
'''
import os
import pytest
from ROOT                   import RDataFrame

from dmu.logging.log_store  import LogStore
from dmu.rdataframe         import utilities as ut
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection      as sel
from rx_selection           import collector      as col
from rx_selection           import truth_matching as tm

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
class Data:
    '''
    data class
    '''
    DATADIR = os.environ['ANADIR'] + '/Data'

    out_dir = '/tmp/tests/rx_selection/selection'
# --------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:selection'     , 10)
    LogStore.set_level('rx_selection:test_selection', 10)

    os.makedirs(Data.out_dir, exist_ok=True)
# --------------------------
def _print_dotted_branches(rdf : RDataFrame) -> None:
    l_col = [ name.c_str() for name in rdf.GetColumnNames() ]
    for name in l_col:
        if '.' not in name:
            continue

        log.debug(name)
# --------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_BuToKpMuMu_MVA'])
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'high'])
def test_read_selection(trigger : str, q2bin : str):
    '''
    Test reading the selection
    '''
    d_sel = sel.selection(trigger=trigger, q2bin=q2bin, process='DATA')
    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
# --------------------------
@pytest.mark.parametrize('sample' , ['Bu_JpsiK_ee_eq_DPC', 'Bu_Kee_eq_btosllball05_DPC', 'DATA*'])
@pytest.mark.parametrize('smeared', [True, False])
@pytest.mark.parametrize('q2bin'  , ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_selection(sample : str, smeared : bool, q2bin : str):
    '''
    Basic test of selection
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = rdf.Range(10_000)

    d_sel = sel.selection(
            trigger=trigger,
            q2bin  =q2bin,
            process=sample,
            smeared=smeared)

    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    rep = rdf.Report()
    df  = ut.rdf_report_to_df(rep)
    df['sample' ] = sample
    df['smeared'] = smeared
    df['q2bin'  ] = q2bin

    col.Collector.add_dataframe(df=df, test_name='selection')

    _print_dotted_branches(rdf)
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_full_selection_electron(sample : str, q2bin : str):
    '''
    Applies full selection to all q2 bins in electron channel
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    gtr     = RDFGetter(sample=sample, trigger=trigger)
    rdf     = gtr.get_rdf()
    rdf     = sel.apply_full_selection(rdf = rdf, trigger=trigger, q2bin=q2bin, process=sample)

    rep     = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()

    assert nentries > 0

    _print_dotted_branches(rdf)
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_Kmumu_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_full_selection_muon(sample : str, q2bin : str):
    '''
    Applies full selection to all q2 bins in muon channel
    '''
    trigger = 'Hlt2RD_BuToKpMuMu_MVA'
    gtr     = RDFGetter(sample=sample, trigger=trigger)
    rdf     = gtr.get_rdf()
    rdf     = sel.apply_full_selection(rdf = rdf, trigger=trigger, q2bin=q2bin, process=sample)

    rep     = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()

    assert nentries > 0

    _print_dotted_branches(rdf)
# --------------------------
def test_override():
    '''
    Will test overriding selection
    Will test overriding selection multiple times
    '''
    mva_cut = 'mva_cmb > 0.1'

    sel.set_custom_selection(d_cut={'bdt' : mva_cut})
    with pytest.raises(sel.MultipleSelectionOverriding):
        sel.set_custom_selection(d_cut={'bdt' : mva_cut})

    q2bin   = 'jpsi'
    sample  = 'DATA*'
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    d_sel   = sel.selection(trigger=trigger, q2bin=q2bin, process=sample)
    cut     = d_sel['bdt']

    assert cut == mva_cut
# --------------------------o
def test_reset_custom_selection():
    '''
    Tests resetting custom selection
    '''
    sel.reset_custom_selection()
    mva_cut = 'mva_cmb > 0.1'

    sel.set_custom_selection(d_cut={'bdt' : mva_cut})
    sel.reset_custom_selection()
    sel.set_custom_selection(d_cut={'bdt' : mva_cut})
# --------------------------
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_KplKplKmn_eq_sqDalitz_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_truth_matching(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='nopid')
    rdf = gtr.get_rdf()

    cut = tm.get_truth(sample)
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    rep = rdf.Report()
    rep.Print()

    assert 20 * fin > ini
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'jpsi', 'psi2', 'high'])
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
