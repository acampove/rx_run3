'''
Unit test for plotter class in dmu.plotting
'''

from importlib.resources import files
from dataclasses         import dataclass

import yaml
import numpy

from ROOT import RDF

from dmu.plotting.plotter import Plotter
#---------------------------------------
@dataclass
class Data:
    '''
    Class used to store shared data
    '''
    nentries = 10000
#---------------------------------------
def _get_rdf(kind):
    '''
    Will return ROOT dataframe with test data
    '''

    d_data = {}
    if   kind == 'class A':
        d_data['x'] = numpy.random.normal(0, 1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(0, 2, size=Data.nentries)
    elif kind == 'class B':
        d_data['x'] = numpy.random.normal(1, 1, size=Data.nentries)
        d_data['y'] = numpy.random.normal(1, 2, size=Data.nentries)
    else:
        raise ValueError(f'Invalid class: {kind}')

    rdf = RDF.FromNumpy(d_data)

    return rdf
#---------------------------------------
def _load_config(kind=None):
    '''
    Will load config file from dmu_data and return dictionary with config
    '''

    cfg_path = files('dmu_data').joinpath(f'plotting/tests/{kind}.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
#---------------------------------------
def _test_simple():
    '''
    Minimal test
    '''
    d_rdf =  { kind : _get_rdf(kind) for kind in ['class A', 'class B'] }

    cfg_dat = _load_config(kind='simple')

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg_dat)
    ptr.run()
#---------------------------------------
def main():
    '''
    Tests start here
    '''
    _test_simple()
#---------------------------------------

if __name__ == '__main__':
    main()
