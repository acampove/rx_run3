#!/usr/bin/env python3
'''
Script in charge of training classifier
'''

import os
import argparse

from importlib.resources import files
from dataclasses         import dataclass

import matplotlib.pyplot as plt
import mplhep
import yaml
from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore

from dmu.ml.train_mva import TrainMva

log = LogStore.add_logger('rx_classifier:train_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class meant to hold data to be shared
    '''

    cfg_dict    : dict
    cfg_name    : str
    max_entries : int
#---------------------------------
def _load_config():
    '''
    Will load YAML file config
    '''

    cfg_path = files('classifier').joinpath(f'data/{Data.cfg_name}.yaml')
    if not os.path.isfile(cfg_path):
        raise FileNotFoundError(f'Could not find: {cfg_path}')

    with open(cfg_path, encoding='utf-8') as ifile:
        Data.cfg_dict = yaml.safe_load(ifile)
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-c', '--cfg_name'   , type=str, help='Kind of config file', required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=10, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1) 
    args = parser.parse_args()

    Data.cfg_name    = args.cfg_name
    Data.max_entries = args.max_entries

    log.setLevel(args.log_level)
#---------------------------------
def _get_rdf(kind=None):
    '''
    Will load and return ROOT dataframe

    Parameters
    ---------------------
    kind (str): kind of dataset to find in config input section
    '''
    tree_name = Data.cfg_dict['dataset']['paths'][kind]['tree_name']
    file_path = Data.cfg_dict['dataset']['paths'][kind]['file_path']

    rdf = RDataFrame(tree_name, file_path)
    rdf = _define_columns(rdf)
    rdf = _apply_selection(rdf, kind)

    if Data.max_entries > 0:
        log.warning(f'Limiting {kind} dataset to {Data.max_entries} entries')
        rdf = rdf.Range(Data.max_entries)

    return rdf
#---------------------------------
def _define_columns(rdf):
    '''
    Will define variables
    '''
    d_var = Data.cfg_dict['dataset']['define']

    log.info('Defining variables')
    for name, expr in d_var.items():
        log.debug(f'---> {name}')
        rdf = rdf.Define(name, expr)

    return rdf
#---------------------------------
def _apply_selection(rdf, kind):
    '''
    Will take ROOT dataframe and kind (bkg or sig)
    Will load selection from config
    Will return dataframe after selection
    '''

    log.info('Applying selection')
    d_cut = Data.cfg_dict['dataset']['selection'][kind]
    for name, cut in d_cut.items():
        log.debug(f'---> {name}')
        rdf = rdf.Filter(cut, name)

    log.info(f'Cutflow for: {kind}')
    rep = rdf.Report()
    rep.Print()

    return rdf
#---------------------------------
def main():
    '''
    Script starts here
    '''

    plt.style.use(mplhep.style.LHCb2)

    _get_args()
    _load_config()

    rdf_bkg = _get_rdf(kind='bkg')
    rdf_sig = _get_rdf(kind='sig')

    trn = TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=Data.cfg_dict)
    trn.run()
#---------------------------------
if __name__ == '__main__':
    main()
