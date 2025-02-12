'''
Module with functions testing MisCalculator
'''

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_data.mis_calculator import MisCalculator

log=LogStore.add_logger('rx_data:test_mis_calculator')

class Data:
    '''
    Class used to share attributes
    '''
    l_must_have = [
            'L1_PE',
            'L2_PE',
            'H_PE']
# ------------------------
def _get_rdf() -> RDataFrame:
    path  = '/home/acampove/external_ssd/Data/main/v5/data_24_magup_24c1_Hlt2RD_BuToKpEE_MVA_0000000000.root'
    rdf   = RDataFrame('DecayTree', path)

    return rdf
# ------------------------
def _check_rdf(rdf : RDataFrame) -> None:
    l_name = [ name.c_str() for name in rdf.GetColumnNames() ]

    success = True
    for must_have in Data.l_must_have:
        if must_have not in l_name:
            success = False
            log.warning(f'Missing {must_have}')

    assert success
# ------------------------
def test_simple():
    '''
    Simplest test of class
    '''
    rdf = _get_rdf()
    cal = MisCalculator(rdf=rdf, trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = cal.get_rdf()

    _check_rdf(rdf)
