'''
Script used to create files with trimmed (few branches) trees containing classifier score
'''

import os
import glob
import argparse

from importlib.resources import files
from dataclasses         import dataclass

import joblib
import numpy
import yaml

from ROOT                  import RDataFrame
from dmu.ml.cv_predict     import CVPredict
from dmu.logging.log_store import LogStore

import dmu.rdataframe.utilities as ut

log = LogStore.add_logger('rx_classifier:apply_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class used to store shared information
    '''
    cfg_name    : str
    cfg_dict    : dict
    max_entries : int
    l_model     : list
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Used to read classifier and write scores to input ntuple, producing output ntuple')
    parser.add_argument('-c', '--cfg_name'   , type=str, help='Kind of config file', required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=10, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    args = parser.parse_args()

    Data.cfg_name    = args.cfg_name
    Data.max_entries = args.max_entries

    log.setLevel(args.log_level)
#---------------------------------
def _load_config():
    '''
    Will load YAML config and set Data.cfg_dict
    '''

    cfg_path = files('classifier').joinpath(f'data/{Data.cfg_name}.yaml')
    if not os.path.isfile(cfg_path):
        raise FileNotFoundError(f'Could not find: {cfg_path}')

    with open(cfg_path, encoding='utf-8') as ifile:
        Data.cfg_dict = yaml.safe_load(ifile)
#---------------------------------
def _add_variables(rdf):
    '''
    Will define extra columns in ROOT dataframe
    according to config
    '''
    d_var_def = Data.cfg_dict['define']

    log.info('Defining variables')
    for name, expr in d_var_def.items():
        rdf = rdf.Define(name, expr)

    return rdf
#---------------------------------
def _get_rdf():
    '''
    Returns a dictionary of dataframes built from paths in config
    '''
    log.info('Getting dataframes')

    d_sample = Data.cfg_dict['samples']
    d_rdf    = {}
    for name, d_info in d_sample.items():
        tree_name = d_info['tree_name']
        file_path = d_info['file_path']

        rdf = RDataFrame(tree_name, file_path)
        rdf = _add_variables(rdf)
        if Data.max_entries > 0:
            rdf = rdf.Range(Data.max_entries)

        d_rdf[name] = rdf

    return d_rdf
#---------------------------------
def _apply_classifier(name, rdf):
    '''
    Takes name of dataset and corresponding ROOT dataframe
    return dataframe with a classifier probability column added
    '''
    cvp     = CVPredict(models=Data.l_model, rdf=rdf)
    arr_prb = cvp.predict()
    l_sig   = [ prb[1] for prb in arr_prb ]
    arr_sig = numpy.array(l_sig)

    name    = Data.cfg_dict['saving']['score']
    rdf     = ut.add_column(rdf, arr_sig, name)

    return rdf
#---------------------------------
def _load_models():
    '''
    Will load classifier models in Data.l_model
    '''

    log.info('Getting models')

    model_path = Data.cfg_dict['model']
    model_wc   = model_path.replace('.pkl', '_*.pkl')
    l_path     = glob.glob(model_wc)
    if len(l_path) == 0:
        log.error(f'No file found in: {model_wc}')
        raise ValueError

    Data.l_model = [ joblib.load(path) for path in l_path ]
#---------------------------------
def _save_rdf(tname, fname, rdf):
    '''
    Will take ROOT dataframe treename and file name (no extension)
    Will save taking a snapshot
    '''
    cls_var = Data.cfg_dict['saving']['score']
    l_var   = Data.cfg_dict['saving']['others'] + [cls_var]
    out_dir = Data.cfg_dict['saving']['out_dir']

    os.makedirs(out_dir, exist_ok=True)
    out_path= f'{out_dir}/{fname}.root'

    log.info(f'Saving to: {out_path}/{tname}')

    rdf.Snapshot(tname, out_path, l_var)
#---------------------------------
def main():
    '''
    Script starts here
    '''

    _get_args()
    _load_config()
    _load_models()

    d_rdf = _get_rdf()

    log.info('Applying classifier')
    for fname, rdf in d_rdf.items():
        tname = Data.cfg_dict['samples'][fname]['tree_name']
        log.info(f'---> {fname}/{tname}')
        rdf = _apply_classifier(fname, rdf)
        _save_rdf(tname, fname, rdf)
#---------------------------------
if __name__ == '__main__':
    main()
