'''
Module with functions testing CVDiagnostics class
'''
import mplhep
import pytest
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from dmu.ml.cv_diagnostics import CVDiagnostics

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:tests:test_cv_diagnostics')
# -------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    plt.style.use(mplhep.style.LHCb2)
# -------------------------------
def test_score_from_model():
    '''
    Measures correlations between target and features + score
    where the score is calculated from the model 
    '''

    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')

    l_model = ut.get_models(rdf_sig, rdf_bkg)
    rdf     = ut.get_rdf(kind='sig')
    cfg     = ut.get_config('ml/tests/diagnostics_from_model.yaml')

    cvd     = CVDiagnostics(models=l_model, rdf=rdf, cfg=cfg)
    cvd.run()
# -------------------------------
def test_score_from_rdf():
    '''
    Measures correlations between target and features + score
    where the score comes from the dataframe
    '''

    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')

    l_model = ut.get_models(rdf_sig, rdf_bkg)
    rdf     = ut.get_rdf(kind='sig')
    cfg     = ut.get_config('ml/tests/diagnostics_from_file.yaml')

    cvd     = CVDiagnostics(models=l_model, rdf=rdf, cfg=cfg)
    cvd.run()
# -------------------------------
