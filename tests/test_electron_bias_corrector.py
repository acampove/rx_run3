'''
Module used to test bias corrections
'''

import os

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                            import RDataFrame
from dmu.logging.log_store           import LogStore
from rx_data.rdf_getter              import RDFGetter
from rx_data.electron_bias_corrector import ElectronBiasCorrector

log=LogStore.add_logger('rx_data:test_electron_bias_corrector')
#-----------------------------------------
class Data:
    '''
    Data class
    '''
    plt_dir = '/tmp/tests/rx_data/electron_bias_corrector'
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:electron_bias_corrector', 10)

    os.makedirs(Data.plt_dir, exist_ok=True)
#-----------------------------------------
def _pick_column(name : str, rdf : RDataFrame) -> bool:
    ctype = rdf.GetColumnType(name)

    if not name.startswith('L1_'):
        return False

    if ctype not in ['Int_t', 'Float_t', 'Double_t', 'int']:
        return False


    return True
#-----------------------------------------
def _get_df() -> pnd.DataFrame:
    RDFGetter.samples = {
        'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
        }

    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Redefine('L1_HASBREMADDED', 'Int_t(L1_HASBREMADDED)')
    rdf = rdf.Range(10)

    l_col  = [ name.c_str() for name in rdf.GetColumnNames() if _pick_column(name.c_str(), rdf) ]
    d_data = rdf.AsNumpy(l_col)
    df     = pnd.DataFrame(d_data)

    return df
#-----------------------------------------
def test_simple():
    '''
    Tests correction of lepton kinematics 
    '''
    df  = _get_df()
    cor = ElectronBiasCorrector()
    for row in df.itertuples():
        row = cor.correct(row=row, name='L1')
#-----------------------------------------
