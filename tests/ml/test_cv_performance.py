'''
Script with functions meant to test CVPerformance
'''
import os

import pytest
import matplotlib.pyplot as plt

from dmu.testing           import utilities as tut
from dmu.logging.log_store import LogStore
from dmu.ml.cv_performance import CVPerformance

log=LogStore.add_logger('dmu:ml:test_cv_performance')
# -------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/stats/cv_performance'
# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------
def test_simple():
    '''
    Simplest test
    '''
    rdf_sig_1 = tut.get_rdf(kind='sig'    )
    rdf_bkg_1 = tut.get_rdf(kind='bkg'    )
    rdf_bkg_2 = tut.get_rdf(kind='bkg_alt')

    l_model_1 = tut.get_models(rdf_sig_1, rdf_bkg_1, name='def')
    l_model_2 = tut.get_models(rdf_sig_1, rdf_bkg_2, name='alt')

    cvp = CVPerformance()
    cvp.plot_roc(
            sig  =rdf_sig_1, bkg=rdf_bkg_1,
            model=l_model_1, name='def', color='red')
    cvp.plot_roc(
            sig  =rdf_sig_1, bkg=rdf_bkg_2,
            model=l_model_2, name='alt', color='blue')

    plt.legend()
    plt.grid()
    plt.savefig(f'{Data.out_dir}/simple.png')
    plt.close()
# -------------------------------
