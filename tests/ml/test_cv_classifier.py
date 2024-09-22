from dmu.ml.cv_classifier import cls


rdf_sig = _get_rdf(kind='sig')
rdf_bkg = _get_rdf(kind='bkg')

def test_simple():
    hyper = _get_conf
model              = cls(**hyper)
model['dset_hash'] = '123'
model['sset_hash'] = '123'
model['random_sd'] = '123'
