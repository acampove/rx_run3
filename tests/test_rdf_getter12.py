'''
Script used to test RDFGetter12 class
'''
import pytest
from dmu.rx_data.rdf_getter12 import RDFGetter12

@pytest.mark.parametrize('sample', ['Bu_Kee_eq_btosllball05_DPC'])
def test_simple(sample : str):
    '''
    Simplest test
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    dset    = '2018'

    gtr = RDFGetter12(
            sample =sample,
            trigger=trigger,
            dset   =dset)

    rdf = gtr.get_rdf()
