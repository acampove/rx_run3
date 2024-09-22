'''
Module containing unit tests for CVClassifier
'''

import os

import numpy
import joblib
import pandas  as pnd

from dmu.logging.log_store import LogStore
from dmu.ml.cv_classifier  import CVClassifier as cls
from dmu.ml.cv_classifier  import CVSameData

import dmu.testing.utilities as ut


# -------------------------------------------------
def _get_train_input():
    '''
    Will return pandas dataframe with features and list of labels
    made from toy data
    '''
    rdf_sig = ut.get_rdf(kind='sig')
    rdf_bkg = ut.get_rdf(kind='bkg')

    cfg         = ut.get_config('ml/tests/train_mva.yaml')
    l_feat_name = cfg['training']['features']

    d_sig = rdf_sig.AsNumpy(l_feat_name)
    d_bkg = rdf_bkg.AsNumpy(l_feat_name)

    df_sig = pnd.DataFrame(d_sig)
    df_bkg = pnd.DataFrame(d_bkg)

    df_ft  = pnd.concat([df_bkg, df_sig], axis=0)

    nbkg    = len(df_bkg)
    nsig    = len(df_sig)

    l_lab   = [0] * nbkg + [1] * nsig
    arr_lab = numpy.array(l_lab)

    return df_ft, arr_lab
# -------------------------------------------------
def test_save_load():
    '''
    Used to save and load class
    '''
    cfg   = ut.get_config('ml/tests/train_mva.yaml')
    hyper = cfg['training']['hyper']

    model      = cls(**hyper)
    model_path = 'tests/ml/CVClassifier/save/model.pkl'
    model_dir  = os.path.dirname(model_path)
    os.makedirs(model_dir, exist_ok=True)

    joblib.dump(model, model_path)

    model = joblib.load(model_path)

    print(model)
# -------------------------------------------------
def test_fit():
    '''
    Test model fitting
    '''

    cfg   = ut.get_config('ml/tests/train_mva.yaml')
    hyper = cfg['training']['hyper']

    df_ft, l_lab = _get_train_input()

    model= cls(**hyper)
    model.fit(df_ft, l_lab)

    model_path = 'tests/ml/CVClassifier/fit/model.pkl'
    model_dir  = os.path.dirname(model_path)
    os.makedirs(model_dir, exist_ok=True)

    joblib.dump(model, model_path)

    print(model)
# -------------------------------------------------
def test_predict():
    '''
    Will test probability prediction 
    '''
    cfg   = ut.get_config('ml/tests/train_mva.yaml')
    hyper = cfg['training']['hyper']

    df_ft, l_lab = _get_train_input()

    model= cls(**hyper)
    model.fit(df_ft, l_lab)

    try:
        _ = model.predict_proba(df_ft)
    except CVSameData:
        pass
# -------------------------------------------------
def main():
    '''
    Tests start here
    '''
    LogStore.set_level('dmu:ml:CVClassifier', 10)

    test_save_load()
    test_fit()
    test_predict()
# -------------------------------------------------
if __name__ == '__main__':
    main()
