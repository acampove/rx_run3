#!/usr/bin/env python3
'''
Script in charge of training classifier
'''

import os
import argparse

from dataclasses import dataclass

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

    cfg_dict : dict
    cfg_name : str
#---------------------------------
def _load_config():
    '''
    Will load YAML file config
    '''

    cfg_path = files('classifier_data').joinpath(f'{Data.cfg_name}.yaml')
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
    parser.add_argument('-c', '--cfg_name' , type=str, help='Kind of config file', required=True)
    args = parser.parse_args()

    Data.cfg_name = args.cfg_name
#---------------------------------
def _get_rdf(kind=None):
    '''
    Will load and return ROOT dataframe

    Parameters
    ---------------------
    kind (str): kind of dataset to find in config input section
    '''
    tree_name = Data.cfg_dict['input'][kind]['tree_name']
    file_path = Data.cfg_dict['input'][kind]['file_path']

    rdf = RDataFrame(tree_name, file_path)
    rdf = _apply_selection(rdf, kind)

    return rdf
#---------------------------------
def _apply_selection(rdf, kind):
    '''
    Will take ROOT dataframe and kind (bkg or sig)
    Will load selection from config
    Will return dataframe after selection
    '''
    d_cut = Data.cfg_dict['dataset']['selection'][kind]
    for name, cut in d_cut.items():
        rdf = rdf.Filter(name, cut)

    log.info(f'Cutflow for: {kind}')
    rep = rdf.Report()
    rep.Print()

    return rdf
#---------------------------------
def main():
    '''
    Script starts here
    '''

    _get_args()
    _load_config()

    rdf_bkg = _get_rdf(kind='bkg')
    rdf_sig = _get_rdf(kind='sig')

    trn = TrainMva(sig=rdf_sig, bkg=rdf_bkg, cfg=Data.cfg_dict)
    trn.run()
#---------------------------------
if __name__ == '__main__':
    main()
