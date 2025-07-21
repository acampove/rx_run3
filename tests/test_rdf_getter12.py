'''
Script used to test RDFGetter12 class
'''
from ROOT import RDataFrame

import pytest
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter12 import RDFGetter12

log=LogStore.add_logger('rx_data:test_rdf_getter12')
# ---------------------------------------------
class Data:
    '''
    Data class
    '''
    l_dset = ['2011', '2012', '2015', '2016', '2017', '2018', 'all']
# ---------------------------------------------
def _check_rdf(rdf : RDataFrame, name : str) -> None:
    nentries = rdf.Count().GetValue()

    log.info('')
    log.info(f'{name:<40}{nentries:<15}')
# ---------------------------------------------
@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC'])
@pytest.mark.parametrize('dset'  , Data.l_dset)
def test_simple(sample : str, dset : str):
    '''
    Simplest test
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    gtr = RDFGetter12(
        sample =sample,
        trigger=trigger,
        dset   =dset)

    rdf = gtr.get_rdf()

    _check_rdf(rdf=rdf, name = f'simple_{sample}_{dset}')
# ---------------------------------------------
