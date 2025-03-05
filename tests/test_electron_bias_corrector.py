'''
Module used to test bias corrections
'''

import os

import numpy
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
    rdf = rdf.Redefine('L1_HASBREMADDED', 'int(L1_HASBREMADDED)')
    rdf = rdf.Redefine('L1_BREMHYPOCOL' , 'int(L1_BREMHYPOCOL)')
    rdf = rdf.Redefine('L1_BREMHYPOROW' , 'int(L1_BREMHYPOROW)')
    rdf = rdf.Range(10)

    l_col  = [ name.c_str() for name in rdf.GetColumnNames() if _pick_column(name.c_str(), rdf) ]
    d_data = rdf.AsNumpy(l_col)
    df     = pnd.DataFrame(d_data)

    return df
#-----------------------------------------
def _check_equal(df_org : pnd.DataFrame, df_cor : pnd.DataFrame, must_differ : bool) -> None:
    equal_cols = numpy.isclose(df_org, df_cor, rtol=0.001)

    if must_differ:
        assert not numpy.all(equal_cols)
    else:
        assert numpy.all(equal_cols)
#-----------------------------------------
def test_skip_correction():
    '''
    Tests without actually doing the correction 
    '''
    df_org = _get_df()
    df_org = df_org.fillna(-1)
    cor    = ElectronBiasCorrector(skip_correction=True)
    df_cor = df_org.apply(lambda row : cor.correct(row, 'L1'), axis=1)

    _check_equal(df_org, df_cor, must_differ = False)
#-----------------------------------------
def test_correction():
    '''
    Tests actually doing the correction 
    '''
    df_org = _get_df()
    df_org = df_org.fillna(-1)
    cor    = ElectronBiasCorrector(skip_correction=False)
    df_cor = df_org.apply(lambda row : cor.correct(row, 'L1'), axis=1)

    _check_equal(df_org, df_cor, must_differ = True)
#-----------------------------------------
