'''
Contains unit tests for CVPredict class
'''

import glob

import joblib

from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva
from dmu.ml.cv_predict     import CVPredict

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:tests:cv_predict')
#--------------------------------------------------------------------
def _get_models(rdf_sig, rdf_bkg):
    '''
    Will train and return models
    '''

    cfg                   = ut.get_config('ml/tests/train_mva.yaml')
    pkl_path              = 'tests/ml/cv_predict/model.pkl'
    plt_dir               = 'tests/ml/cv_predict'
    cfg['saving']['path'] = pkl_path
    cfg['plotting']['val_dir'] = plt_dir
    cfg['plotting']['features']['saving']['plt_dir'] = plt_dir

    obj= TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=cfg)
    obj.run()

    pkl_wc     = pkl_path.replace('.pkl', '_*.pkl')
    l_pkl_path = glob.glob(pkl_wc)
    l_model    = [ joblib.load(pkl_path) for pkl_path in l_pkl_path ]

    return l_model
#--------------------------------------------------------------------
def test_non_overlap():
    '''
    Tests prediction when input dataset is different from training one
    '''

    LogStore.set_level('dmu:ml:cv_predict', 10)
    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')
    l_model = _get_models(rdf_sig, rdf_bkg)

    rdf     = ut.get_rdf(kind='sig')
    cvp     = CVPredict(models=l_model, rdf=rdf)
    cvp.predict()
#--------------------------------------------------------------------
def test_overlap():
    '''
    Tests prediction when input dataset is same as training one
    '''
    LogStore.set_level('dmu:ml:cv_predict', 10)

    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')
    l_model = _get_models(rdf_sig, rdf_bkg)

    cvp     = CVPredict(models=l_model, rdf=rdf_sig)
    cvp.predict()
#--------------------------------------------------------------------
