'''
Contains unit tests for CVPredict class
'''

import glob

import joblib
import pytest

from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva
from dmu.ml.cv_predict     import CVPredict

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:tests:cv_predict')
#--------------------------------------------------------------------
class Data:
    '''
    Will store data to be shared
    '''
    rdf_sig = None
    rdf_bkg = None
#--------------------------------------------------------------------
@pytest.fixture(scope='module', autouse=True)
def _initialize():
    Data.rdf_sig = ut.get_rdf(kind='sig')
    Data.rdf_bkg = ut.get_rdf(kind='bkg')
#--------------------------------------------------------------------
def _get_models():
    '''
    Will train and return models
    '''
    cfg                   = ut.get_config('ml/tests/train_mva.yaml')
    pkl_path              = 'tests/ml/cv_predict/model.pkl'
    plt_dir               = 'tests/ml/cv_predict'
    cfg['saving']['path'] = pkl_path
    cfg['plotting']['val_dir'] = plt_dir
    cfg['plotting']['features']['saving']['plt_dir'] = plt_dir

    obj= TrainMva(sig=Data.rdf_sig, bkg=Data.rdf_bkg, cfg=cfg)
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
    l_model = _get_models()
    rdf     = ut.get_rdf(kind='sig')

    cvp     = CVPredict(models=l_model, rdf=rdf)
    arr_scr = cvp.predict()
#--------------------------------------------------------------------
def main():
    '''
    Tests start here
    '''
    _initialize()

    test_non_overlap()
#--------------------------------------------------------------------
if __name__ == '__main__':
    main()
