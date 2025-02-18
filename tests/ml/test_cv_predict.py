'''
Contains unit tests for CVPredict class
'''

import glob
import pytest

import joblib
import numpy

from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva
from dmu.ml.cv_predict     import CVPredict

import dmu.testing.utilities as ut

log = LogStore.add_logger('dmu:ml:tests:cv_predict')
# -------------------------------------------------
class Data:
    '''
    Class used to share attributes
    '''

    out_dir = '/tmp/dmu/tests/ml/cv_predict'
#--------------------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _set_logs():
    LogStore.set_level('dmu:ml:cv_predict', 10)
    LogStore.set_level('dmu:ml:utilities' , 10)
#--------------------------------------------------------------------
def _check_probabilities(arr_prb : numpy) -> None:
    n_above = int(numpy.sum(arr_prb > 1))
    n_below = int(numpy.sum(arr_prb < 0))

    if n_above != 0:
        log.error(f'Found {n_above} elements above 1')

    if n_below != 0:
        log.error(f'Found {n_above} elements below 0')

    assert n_above == 0
    assert n_below == 0
#--------------------------------------------------------------------
def _get_models(rdf_sig, rdf_bkg):
    '''
    Will train and return models
    '''

    cfg                   = ut.get_config('ml/tests/train_mva.yaml')
    pkl_path              = f'{Data.out_dir}/model.pkl'
    plt_dir               = f'{Data.out_dir}/cv_predict'
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
def test_patch_and_tag():
    '''
    Tests prediction when input dataset needs to be cleaned
    '''
    log.info('')

    LogStore.set_level('dmu:ml:cv_predict', 10)
    LogStore.set_level('dmu:ml:train_mva' , 20)

    rdf_sig = ut.get_rdf(kind='sig', add_nans=True)
    rdf_bkg = ut.get_rdf(kind='bkg', repeated=True)
    l_model = _get_models(rdf_sig, rdf_bkg)

    log.info('Predicting')

    rdf     = ut.get_rdf(kind='sig', add_nans=True)
    cvp     = CVPredict(models=l_model, rdf=rdf)
    arr_prb = cvp.predict()

    _check_probabilities(arr_prb)
