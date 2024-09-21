'''
Unit test for Mva class
'''

from dataclasses     import dataclass

import numpy
from ROOT import RDF 

from log_store       import log_store
from data_checks.mva import Mva


log = log_store.add_logger('data_checks:test_mva')
# -------------------------------
@dataclass
class Data:
    '''
    Class storing shared data
    '''
    nentries = 1000
# -------------------------------
def _get_rdf(kind : str | None = None):
    '''
    Return ROOT dataframe with toy data
    '''
    d_data = {}
    if   kind == 'mc':
        d_data['x'] = numpy.random.uniform(0, 1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(0, 1, size=Data.nentries)
    elif kind == 'dt':
        d_data['x'] = numpy.random.exponential(1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(1, 1, size=Data.nentries)
    else:
        log.error(f'Invalid kind: {kind}')
        raise ValueError

    rdf = RDF.FromNumpy(d_data)

    return rdf
# -------------------------------
def _get_config():
    return {'training' : {'nfold' : 10}}
# -------------------------------
def _test_train():
    '''
    Test training
    '''
    rdf_mc = _get_rdf(kind='mc')
    rdf_dt = _get_rdf(kind='dt')
    cfg    = _get_config()

    obj    = Mva(rdf_mc, rdf_dt, cfg)
    rdf_dt = obj.get_rdf(mva_col='BDT', kind='dt')
    rdf_mc = obj.get_rdf(mva_col='BDT', kind='mc')
# -------------------------------
def main():
    '''
    Script starts here
    '''
    _test_train()
# -------------------------------
if __name__ == '__main__':
    main()
