'''
Script with functions needed to test functions in selection.py
'''
import pytest

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel

log=LogStore.add_logger('rx_selection:test_selection')
# --------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    RDFGetter.samples = {
            'main'      : '/home/acampove/external_ssd/Data/samples/main.yaml',
            'mva'       : '/home/acampove/external_ssd/Data/samples/mva.yaml',
            'hop'       : '/home/acampove/external_ssd/Data/samples/hop.yaml',
            'cascade'   : '/home/acampove/external_ssd/Data/samples/cascade.yaml',
            'jpsi_misid': '/home/acampove/external_ssd/Data/samples/jpsi_misid.yaml',
            'ecalo_bias': '/home/acampove/external_ssd/Data/samples/ecalo_bias.yaml',
            }

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
# --------------------------o
