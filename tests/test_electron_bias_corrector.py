'''
Module used to test bias corrections
'''

import pytest
from ROOT import RDataFrame

from dmu.logging.log_store           import LogStore
from rx_data.rdf_getter              import RDFGetter
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:test_electron_bias_corrector')
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:electron_bias_corrector', 10)
#-----------------------------------------
def _get_rdf() -> RDataFrame:
    RDFGetter.samples = {
        'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
        }

    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Range(10)

    return rdf
#-----------------------------------------
def test_simle():
    '''
    Starts here
    '''
    rdf = _get_rdf()

    cor = ElectronBiasCorrector(rdf=rdf)
    rdf = cor.get_rdf()
#-----------------------------------------
