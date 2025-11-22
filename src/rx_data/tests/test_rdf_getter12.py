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
    l_samp = [
        #'Bu_Kmumu_eq_btosllball05_DPC',
        #'Bu_JpsiK_mm_eq_DPC',
        #'Bu_JpsiK_ee_eq_DPC',
        'Bu_Kee_eq_btosllball05_DPC']
# ---------------------------------------------
def _check_rdf(rdf : RDataFrame, name : str) -> None:
    nentries = rdf.Count().GetValue()

    assert nentries > 0

    log.info('')
    log.info(f'{name:<40}{nentries:<15}')
# ---------------------------------------------
@pytest.mark.parametrize('sample', Data.l_samp)
@pytest.mark.parametrize('dset'  , Data.l_dset)
def test_simple(sample : str, dset : str):
    '''
    Simplest test
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA' if 'ee_eq' in sample else 'Hlt2RD_BuToKpMuMu_MVA'

    gtr = RDFGetter12(
        sample =sample,
        trigger=trigger,
        dset   =dset)

    rdf = gtr.get_rdf()

    _check_rdf(rdf=rdf, name = f'simple_{sample}_{dset}')
# ---------------------------------------------
@pytest.mark.parametrize('sample', Data.l_samp)
@pytest.mark.parametrize('dset'  , Data.l_dset)
def test_add_selection(sample : str, dset : str):
    '''
    Simplest test
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA' if 'ee_eq' in sample else 'Hlt2RD_BuToKpMuMu_MVA'
    d_sel   = {
        'bdt' : 'mva_cmb > 0.5 & mva_prc > 0.5',
        'q2'  : 'q2_track > 14300000'}

    with RDFGetter12.add_selection(d_sel = d_sel):
        gtr = RDFGetter12(
            sample =sample,
            trigger=trigger,
            dset   =dset)

        rdf = gtr.get_rdf()

    _check_rdf(rdf=rdf, name = f'simple_{sample}_{dset}')
# ---------------------------------------------
