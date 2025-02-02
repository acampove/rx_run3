'''
Script used to create files with trimmed (few branches) trees containing classifier score
'''

import os
import glob
import argparse

from dataclasses         import dataclass

import numexpr
import joblib
import numpy
import yaml

from ROOT                  import RDataFrame, RDF
from dmu.ml.cv_predict     import CVPredict
from dmu.logging.log_store import LogStore
from rx_selection          import selection as sel

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

    if not os.path.isfile(Data.cfg_name):
        raise FileNotFoundError(f'Could not find: {Data.cfg_name}')

    with open(Data.cfg_name, encoding='utf-8') as ifile:
        Data.cfg_dict = yaml.safe_load(ifile)
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
        if Data.max_entries > 0:
            rdf = rdf.Range(Data.max_entries)

        nentries = rdf.Count().GetValue()
        log.info(f'Using {nentries} entries for sample {name}')

        d_rdf[name] = rdf

    return d_rdf
#---------------------------------
def _set_loggers():
    LogStore.set_level('dmu:ml:cv_predict', 20)
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
    l_var   = Data.cfg_dict['saving']['branches']
    out_dir = Data.cfg_dict['saving']['out_dir']

    os.makedirs(out_dir, exist_ok=True)
    out_path= f'{out_dir}/{fname}.root'

    log.info(f'Saving to: {out_path}/{tname}')

    if l_var is None:
        rdf.Snapshot(tname, out_path)
    else:
        rdf.Snapshot(tname, out_path, l_var)
#---------------------------------
def main():
    '''
    Script starts here
    '''

    _get_args()
    _load_config()
    _set_loggers()

    d_rdf = _get_rdf()

    log.info('Applying classifier')
    for fname, rdf in d_rdf.items():
        tname = Data.cfg_dict['samples'][fname]['tree_name']
        log.info(f'---> {fname}/{tname}')
        rdf = _apply_classifier(rdf)
        _save_rdf(tname, fname, rdf)
#---------------------------------
if __name__ == '__main__':
    main()
