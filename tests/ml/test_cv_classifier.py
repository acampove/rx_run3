'''
Module containing unit tests for CVClassifier
'''

import os

import joblib

from dmu.ml.cv_classifier import CVClassifier as cls

import dmu.testing.utilities as ut

#rdf_sig = _get_rdf(kind='sig')
#rdf_bkg = _get_rdf(kind='bkg')

# -------------------------------------------------
def test_save_load():
    '''
    Used to save and load class
    '''
    cfg   = ut.get_config('ml/tests/train_mva.yaml')
    hyper = cfg['training']['hyper']

    model              = cls(**hyper)
    model['dset_hash'] = '123'
    model['sset_hash'] = '321'

    model_path = 'tests/ml/CVClassifier/save/model.pkl'
    model_dir  = os.path.dirname(model_path)
    os.makedirs(model_dir, exist_ok=True)

    joblib.dump(model, model_path)

    model = joblib.load(model_path)

    dh = model['dset_hash']
    sh = model['sset_hash']

    assert dh == '123'
    assert sh == '321'
# -------------------------------------------------
def main():
    '''
    Tests start here
    '''
    test_save_load()
# -------------------------------------------------
if __name__ == '__main__':
    main()
