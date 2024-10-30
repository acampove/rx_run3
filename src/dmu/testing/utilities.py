'''
Module containing utility functions needed by unit tests
'''

from typing              import Union
from dataclasses         import dataclass
from importlib.resources import files

import pandas as pnd
import numpy
import yaml
from ROOT import RDF, RDataFrame

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:testing:utilities')
# -------------------------------
@dataclass
class Data:
    '''
    Class storing shared data
    '''
    nentries = 3000
# -------------------------------
def _double_data(d_data : dict) -> dict:
    df_1   = pnd.DataFrame(d_data)
    df_2   = pnd.DataFrame(d_data)

    df     = pnd.concat([df_1, df_2], axis=0)

    d_data = { name : df[name].to_numpy() for name in df.columns }

    return d_data
# -------------------------------
def _add_nans(d_data : dict) -> dict:
    return d_data
# -------------------------------
def get_rdf(kind : Union[str,None] = None,
            repeated : bool        = False,
            add_nans : bool        = False):
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

    if repeated:
        d_data = _double_data(d_data)

    if add_nans:
        d_data = _add_nans(d_data)

    rdf = RDF.FromNumpy(d_data)

    return rdf
# -------------------------------
def get_config(name : Union[str,None] = None):
    '''
    Takes path to the YAML config file, after `dmu_data`
    Returns dictionary with config
    '''
    if name is None:
        raise ValueError('Name not pased')

    cfg_path = files('dmu_data').joinpath(name)
    cfg_path = str(cfg_path)
    with open(cfg_path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    return d_cfg
# -------------------------------
