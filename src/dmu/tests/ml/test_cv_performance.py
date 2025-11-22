'''
Script with functions meant to test CVPerformance
'''
import os
import math

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
    out_dir = '/tmp/tests/dmu/ml/cv_performance'
# -------------------------------
def test_simple():
    '''
    Simplest test
    '''
    out_dir   = f'{Data.out_dir}/simple'
    os.makedirs(out_dir, exist_ok=True)

    rdf_sig_1 = tut.get_rdf(kind='sig'    )
    rdf_bkg_1 = tut.get_rdf(kind='bkg'    )
    rdf_bkg_2 = tut.get_rdf(kind='bkg_alt')

    l_model_1, auc_11 = tut.get_models(rdf_sig_1, rdf_bkg_1, out_dir=f'{out_dir}/def')
    l_model_2, auc_12 = tut.get_models(rdf_sig_1, rdf_bkg_2, out_dir=f'{out_dir}/alt')

    cvp    = CVPerformance()
    auc_21 = cvp.plot_roc(sig  =rdf_sig_1, bkg=rdf_bkg_1, model=l_model_1, name='def', color='red')
    auc_22 = cvp.plot_roc(sig  =rdf_sig_1, bkg=rdf_bkg_2, model=l_model_2, name='alt', color='blue')

    plt.legend()
    plt.grid()
    plt.savefig(f'{out_dir}/performance.png')
    plt.close()

    assert math.isclose(auc_11, auc_21, rel_tol=1e-5)
    assert math.isclose(auc_12, auc_22, rel_tol=1e-5)
# -------------------------------
