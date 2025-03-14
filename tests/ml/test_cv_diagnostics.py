'''
Module with functions testing CVDiagnostics class
'''
from dmu.logging.log_store import LogStore
from dmu.ml.cv_predict     import CVPredict
from dmu.ml.cv_diagnostics import CVDiagnostics

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:tests:test_cv_diagnostics')
# -------------------------------
def test_correlations():
    '''
    Measures correlations between signal probability and other variables
    '''
    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')

    l_model = ut.get_models(rdf_sig, rdf_bkg)
    rdf_sig = ut.get_rdf(kind='sig')
    cfg     = ut.get_config('ml/tests/cv_diagnostics.yaml')

    cvd     = CVDiagnostics(models=l_model, rdf=rdf_sig, cfg=cfg)
    cvd.run()
# -------------------------------
