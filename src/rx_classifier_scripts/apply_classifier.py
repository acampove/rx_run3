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
    index       : int
    cfg_name    : str
    cfg_dict    : dict
    max_entries : int
    l_model     : list
    log_level   : int
    dry_run     : bool
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Used to read classifier and write scores to input ntuple, producing output ntuple')
    parser.add_argument('-c', '--cfg_name'   , type=str, help='Kind of config file', required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=20, choices=[10, 20, 30])
    parser.add_argument('-i', '--index'      , type=int, help='Input index')
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    parser.add_argument('-d', '--dry_run'    ,           help='Dry run', action='store_true')
    args = parser.parse_args()

    Data.index       = args.index
    Data.cfg_name    = args.cfg_name
    Data.max_entries = args.max_entries
    Data.log_level   = args.log_level
    Data.dry_run     = args.dry_run
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
def _get_rdf(file_path : str) -> RDataFrame:
    '''
    Returns a dictionary of dataframes built from paths in config
    '''
    log.info('Getting dataframes')

    rdf = RDataFrame('DecayTree', file_path)
    if Data.max_entries > 0:
        rdf = rdf.Range(Data.max_entries)

    nentries = rdf.Count().GetValue()
    log.info(f'Using {nentries} entries for sample {file_path}')

    return rdf
#---------------------------------
def _set_loggers():
    LogStore.set_level('dmu:ml:cv_predict'             , Data.log_level)
    LogStore.set_level('rx_classifier:apply_classifier', Data.log_level)
#---------------------------------
def _get_q2_indexer() -> str:
    '''
    Returns a string that depends on Jpsi_M.
    When evaluated it gives
    - 0 for resonant
    - 1 for low
    - 2 for central
    - 3 for high

    q2 bin
    '''
    sel_cfg  = sel.load_selection_config()
    d_q2_cut = sel_cfg['q2_common']

    low_cut  = d_q2_cut['low'    ]
    cen_cut  = d_q2_cut['central']
    hig_cut  = d_q2_cut['high'   ]

    cond     = f'1 * ({low_cut}) + 2 * ({cen_cut}) + 3 * ({hig_cut})'
    cond     = cond.replace('&&', '&')

    log.debug(f'Using q2 indexer: {cond}')

    return cond
# ----------------------------------------
def _q2_scores_from_rdf(rdf : RDataFrame, path : str) -> numpy.ndarray:
    l_pkl  = glob.glob(f'{path}/*.pkl')
    npkl   = len(l_pkl)
    if npkl == 0:
        raise ValueError(f'No pickle files found in {path}')

    log.info(f'Using {npkl} pickle files from: {path}')

    l_model = [ joblib.load(pkl_path) for pkl_path in l_pkl ]

    cvp     = CVPredict(models=l_model, rdf=rdf)
    arr_prb = cvp.predict()

    return arr_prb
# ----------------------------------------
def _get_full_q2_scores(
        low     : numpy.ndarray,
        central : numpy.ndarray,
        high    : numpy.ndarray,
        jpsi_m  : numpy.ndarray) -> numpy.ndarray:
    '''
    Takes arrays of MVA in 3 q2 bins, as well as array of jpsi mass.
    Returns array of mva score correspoinding to right q2 bin.
    '''

    q2_cond     = _get_q2_indexer()
    arr_ind     = numexpr.evaluate(q2_cond, local_dict={'Jpsi_M' : jpsi_m})

    # Resonant q2 bin will pick up central-q2 scores
    arr_all_q2  = numpy.array([central, low, central, high])
    arr_full_q2 = numpy.choose(arr_ind, arr_all_q2)

    return arr_full_q2
# ----------------------------------------
def _scores_from_rdf(rdf : RDataFrame, d_path : dict[str,str]) -> numpy.ndarray:
    arr_jpsi_m  = rdf.AsNumpy(['Jpsi_M'])['Jpsi_M']

    # For dry runs return Jpsi mass as score. Won't be used or saved anyway
    if Data.dry_run:
        return arr_jpsi_m

    arr_low     = _q2_scores_from_rdf(rdf, d_path['low'    ])
    arr_central = _q2_scores_from_rdf(rdf, d_path['central'])
    arr_high    = _q2_scores_from_rdf(rdf, d_path['high'   ])

    arr_mva     = _get_full_q2_scores(low=arr_low, central=arr_central, high=arr_high, jpsi_m=arr_jpsi_m)

    return arr_mva
# ----------------------------------------
def _apply_classifier(rdf : RDataFrame) -> RDataFrame:
    '''
    Takes name of dataset and corresponding ROOT dataframe
    return dataframe with a classifier probability column added
    '''

    if 'mva' not in Data.cfg_dict:
        raise ValueError('Cannot find MVA section in config')

    d_mva_kind = Data.cfg_dict['mva']
    if len(d_mva_kind) == 0:
        raise ValueError('No MVAs found, skipping addition')

    nmva = len(d_mva_kind)
    log.debug(f'Found {nmva} kinds of MVA scores')

    d_mva_score = { f'mva_{name}' : _scores_from_rdf(rdf, d_path) for name, d_path in d_mva_kind.items() }

    d_data      = rdf.AsNumpy(['RUNNUMBER', 'EVENTNUMBER'])
    d_data.update(d_mva_score)
    rdf         = RDF.FromNumpy(d_data)

    return rdf
#---------------------------------
def _get_paths() -> list[str]:
    inp_dir = Data.cfg_dict['input_dir']
    file_wc = f'{inp_dir}/**/*_sample.root'
    l_path  = glob.glob(file_wc, recursive=True)

    nfile   = len(l_path)
    if nfile == 0:
        raise ValueError(f'No file found in: {file_wc}')

    log.info(f'Found {nfile} files')

    if Data.index is None:
        return l_path

    if Data.index + 1 > nfile:
        raise ValueError(f'Cannot run for index {Data.index}')

    l_path = [l_path[Data.index]]
    log.info(f'Restricting run to input with index: {Data.index}')

    return l_path
#---------------------------------
def main():
    '''
    Script starts here
    '''

    _get_args()
    _load_config()
    _set_loggers()

    log.info('Applying classifier')
    l_file_path = _get_paths()
    for file_path in l_file_path:
        rdf = _get_rdf(file_path)
        rdf = _apply_classifier(rdf)

        out_path = file_path.replace('_sample.root', '_mva.root')
        log.info(f'Saving to: {out_path}')

        if not Data.dry_run:
            rdf.Snapshot('DecayTree', out_path)

        log.info('')
#---------------------------------
if __name__ == '__main__':
    main()
