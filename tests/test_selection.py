'''
Script with functions needed to test functions in selection.py
'''
import os
import pytest

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
class Data:
    '''
    data class
    '''
    DATADIR = os.environ['DATADIR']
# --------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_selection:selection'     , 10)
    LogStore.set_level('rx_selection:test_selection', 10)
# --------------------------
def _print_dotted_branches(rdf : RDataFrame) -> None:
    l_col = [ name.c_str() for name in rdf.GetColumnNames() ]
    for name in l_col:
        if '.' not in name:
            continue

        log.debug(name)
# --------------------------
@pytest.mark.parametrize('analysis', ['EE', 'MM'])
@pytest.mark.parametrize('q2bin'   , ['low', 'central', 'high'])
def test_read_selection(analysis : str, q2bin : str):
    '''
    Test reading the selection
    '''
    d_sel = sel.selection(project='RK', analysis=analysis, q2bin=q2bin, process='DATA')
    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
# --------------------------
@pytest.mark.parametrize('sample', ['Bu_JpsiK_ee_eq_DPC', 'DATA*'])
def test_selection(sample : str):
    '''
    Applies selection
    '''
    gtr = RDFGetter(sample=sample, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Range(10_000)

    d_sel = sel.selection(project='RK', analysis='EE', q2bin='jpsi', process=sample)
    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    rep = rdf.Report()
    rep.Print()

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
    rdf     = sel.apply_full_selection(rdf = rdf, project='RK', trigger=trigger, q2bin=q2bin, process=sample)

    rep     = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()

    assert nentries > 0

    _print_dotted_branches(rdf)
# --------------------------o
@pytest.mark.parametrize('sample', ['Bu_Kmumu_eq_btosllball05_DPC', 'DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('q2bin' , ['low', 'central', 'jpsi', 'psi2', 'high'])
def test_full_selection_muon(sample : str, q2bin : str):
    '''
    Applies full selection to all q2 bins in muon channel
    '''
    trigger = 'Hlt2RD_BuToKpMuMu_MVA'
    gtr     = RDFGetter(sample=sample, trigger=trigger)
    rdf     = gtr.get_rdf()
    rdf     = sel.apply_full_selection(rdf = rdf, project='RK', trigger=trigger, q2bin=q2bin, process=sample)

    rep     = rdf.Report()
    rep.Print()

    nentries = rdf.Count().GetValue()

    assert nentries > 0

    _print_dotted_branches(rdf)
# --------------------------o
