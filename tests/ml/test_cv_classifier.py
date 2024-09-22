from dmu.ml.cv_classifier import cls

import dmu.testing.utilities as ut

#rdf_sig = _get_rdf(kind='sig')
#rdf_bkg = _get_rdf(kind='bkg')

# -------------------------------------------------
def test_simple():
    '''
    Used to do simplest test 
    '''
    hyper = ut.get_config('ml/tests/train_mva.yaml')

    model              = cls(**hyper)
    model['dset_hash'] = '123'
    model['sset_hash'] = '123'
    model['random_sd'] = '123'
# -------------------------------------------------
def main():
    '''
    Tests start here
    '''
    test_simple()
# -------------------------------------------------
if __name__ == '__main__':
    main()
