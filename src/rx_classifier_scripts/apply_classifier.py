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
    max_path    = 700
    force_new   : bool
    sample      : str
    trigger     : str
    cfg_path    : str
    cfg_dict    : dict
    max_entries : int
    l_model     : list
    log_level   : int
    dry_run     : bool
    l_part      : list[int]
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Used to read classifier and write scores to input ntuple, producing output ntuple')
    parser.add_argument('-c', '--cfg_path'   , type=str, help='Path to yaml file with configuration'        , required=True)
    parser.add_argument('-s', '--sample'     , type=str, help='Sample name, meant to exist inside input_dir', required=True)
    parser.add_argument('-t', '--trigger'    , type=str, help='HLT trigger'                                 , required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=20, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    parser.add_argument('-d', '--dry_run'    ,           help='Dry run', action='store_true')
    parser.add_argument('-f', '--force_new'  ,           help='Will remake outputs, even if they already exist', action='store_true')
    parser.add_argument('-p', '--partition'  , nargs= 2, help='Partition, two integers, such that the input is split into nparts and the script processes one of them', default=[0,1])
    args = parser.parse_args()

    Data.sample      = args.sample
    Data.trigger     = args.trigger
    Data.cfg_path    = args.cfg_path
    Data.max_entries = args.max_entries
    Data.log_level   = args.log_level
    Data.dry_run     = args.dry_run
    Data.force_new   = args.force_new
    Data.l_part      = _parts_from_partition(args.partition)
#---------------------------------
def _parts_from_partition(l_partition : list[str]) -> list[int]:
    ipart = int(l_partition[0])
    npart = int(l_partition[1])

    return [ipart, npart]
#---------------------------------
def _load_config():
    '''
    Will load YAML config and set Data.cfg_dict
    '''

    if not os.path.isfile(Data.cfg_path):
        raise FileNotFoundError(f'Could not find: {Data.cfg_path}')

    with open(Data.cfg_path, encoding='utf-8') as ifile:
        Data.cfg_dict = yaml.safe_load(ifile)
#---------------------------------
def _get_rdf(l_file_path : list[str]) -> RDataFrame:
    '''
    Returns a dictionary of dataframes built from paths in config
    '''
    log.info('Getting dataframes')

    rdf = RDataFrame('DecayTree', l_file_path)
    if Data.max_entries > 0:
        rdf = rdf.Range(Data.max_entries)

    nentries = rdf.Count().GetValue()
    log.info(f'Using {nentries} entries for {Data.sample}/{Data.trigger}')

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
def _partition_paths(l_path : list[str]) -> list[str]:
    [ipart, npart] = Data.l_part
    log.info(f'Partitioning with {ipart}/{npart}')

    l_part = numpy.array_split(l_path, npart)

    return l_part[ipart]
#---------------------------------
def _get_paths() -> list[str]:
    if 'samples' not in Data.cfg_dict:
        raise ValueError('samples entry not found')

    samples_path = Data.cfg_dict['samples']
    with open(samples_path, encoding='utf-8') as ifile:
        d_sample = yaml.safe_load(ifile)

    if Data.sample not in d_sample:
        raise ValueError(f'Cannot find {Data.sample} among samples')

    d_trigger = d_sample[Data.sample]
    if Data.trigger not in d_trigger:
        raise ValueError(f'Cannot find {Data.sample} among triggers for sample {Data.sample}')

    l_path = d_trigger[Data.trigger]
    l_path = _partition_paths(l_path)
    npath  = len(l_path)

    if npath > Data.max_path:
        raise ValueError(f'Cannot process more than {Data.max_path} paths, requested {npath}')

    log.info(f'Found {npath} paths for {Data.sample}/{Data.trigger}')

    return l_path
#---------------------------------
def _get_out_path(l_input_path : list[str]) -> str:
    input_path = l_input_path[0]
    name       = os.path.basename(input_path)
    l_part     = name.split('_')
    l_part     = l_part[:-1]
    name       = '_'.join(l_part)

    out_dir = Data.cfg_dict['output']
    [ipart, npart] = Data.l_part

    os.makedirs(out_dir, exist_ok=True)

    return f'{out_dir}/{name}_{ipart:03}_{npart:03}.root'
#---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _load_config()
    _set_loggers()

    l_file_path = _get_paths()
    out_path    = _get_out_path(l_file_path)

    if os.path.isfile(out_path):
        log.info('Output already found, skipping')
        return

    rdf = _get_rdf(l_file_path)

    log.info('Applying classifier')
    rdf = _apply_classifier(rdf)

    if not Data.dry_run:
        log.info(f'Saving to: {out_path}')
        rdf.Snapshot('DecayTree', out_path)

    log.info('')
#---------------------------------
if __name__ == '__main__':
    main()
