'''
Unit test for Mva class
'''


from dataclasses         import dataclass
from importlib.resources import files

import numpy
import yaml
from ROOT import RDF

from dmu.logging.log_store import LogStore
from dmu.ml.train_mva      import TrainMva

log = LogStore.add_logger('dmu:ml:test_train_mva')
# -------------------------------
@dataclass
class Data:
    '''
    Class storing shared data
    '''
    nentries = 10000
# -------------------------------
def _get_rdf(kind : str | None = None):
    '''
    Return ROOT dataframe with toy data
    '''
    d_data = {}
    if   kind == 'sig':
        d_data['w'] = numpy.random.normal(0, 1, size=Data.nentries)
        d_data['x'] = numpy.random.normal(0, 1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(0, 1, size=Data.nentries)
        d_data['z'] = numpy.random.normal(0, 1, size=Data.nentries)
    elif kind == 'bkg':
        d_data['w'] = numpy.random.normal(1, 1, size=Data.nentries)
        d_data['x'] = numpy.random.normal(1, 1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(1, 1, size=Data.nentries)
        d_data['z'] = numpy.random.normal(1, 1, size=Data.nentries)
    else:
        log.error(f'Invalid kind: {kind}')
        raise ValueError

    rdf = RDF.FromNumpy(d_data)

    return rdf
# -------------------------------
def _get_config():
    cfg_path = files('dmu_data').joinpath('ml/tests/train_mva.yaml')
    cfg_path = str(cfg_path)
    with open(cfg_path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    return d_cfg
# -------------------------------
def _test_train():
    '''
    Test training
    '''
    rdf_sig = _get_rdf(kind='sig')
    rdf_bkg = _get_rdf(kind='bkg')
    cfg     = _get_config()

    obj= TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=cfg)
    obj.run()
# -------------------------------
def main():
    '''
    Script starts here
    '''
    LogStore.set_level('data_checks:train_mva', 10)

    _test_train()
# -------------------------------
if __name__ == '__main__':
    main()
