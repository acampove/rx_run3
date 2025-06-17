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

from ROOT                  import RDataFrame, RDF
from dmu.ml.cv_predict     import CVPredict
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

log = LogStore.add_logger('rx_classifier:apply_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class used to store shared information
    '''
    max_path    = 700
    ana_dir     = os.environ['ANADIR']
    version     : str
    force_new   : bool
    sample      : str
    trigger     : str
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
    parser.add_argument('-v', '--version'    , type=str, help='Version of classifier'                       , required=True)
    parser.add_argument('-s', '--sample'     , type=str, help='Sample name'                                 , required=True)
    parser.add_argument('-t', '--trigger'    , type=str, help='HLT trigger'                                 , required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=20, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    parser.add_argument('-d', '--dry_run'    ,           help='Dry run', action='store_true')
    parser.add_argument('-f', '--force_new'  ,           help='Will remake outputs, even if they already exist', action='store_true')
    args = parser.parse_args()

    Data.version     = args.version
    Data.sample      = args.sample
    Data.trigger     = args.trigger
    Data.log_level   = args.log_level
    Data.max_entries = args.max_entries
    Data.dry_run     = args.dry_run
    Data.force_new   = args.force_new
#---------------------------------
def _filter_rdf(rdf : RDataFrame) -> RDataFrame:
    '''
    Applies any filter before running prediction
    '''

    if Data.max_entries > 0:
        rdf = rdf.Range(Data.max_entries)

    nentries = rdf.Count().GetValue()
    log.info(f'Using {nentries} entries for {Data.sample}/{Data.trigger}')

    return rdf
#---------------------------------
def _add_muon_columns(rdf : RDataFrame) -> RDataFrame:
    '''
    Defines columns in dataframe, needed only for muon channel:

    - Columns with brem correction, that exist in electron channel, but in the muon channel are the same as the default ones
    '''
    if 'MuMu' not in Data.trigger:
        log.debug('Not defining muon columns before prediction for: {Data.trigger}')
        return rdf

    log.info(f'Defining muon columns before prediction for: {Data.trigger}')
    l_var = [
            'B_PT',
            'Jpsi_PT',
            'B_DIRA_OWNPV',
            'Jpsi_DIRA_OWNPV',
            'L1_PT',
            'L2_PT',
            'B_M',
            ]

    for var in l_var:
        # The underscore is needed due to:
        #
        # - ROOT does not allow for branches with periods
        # - Periods are replaced with underscore in CVPredict tool anyway.
        rdf = rdf.Define(f'brem_track_2_{var}_brem_track_2', var)

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

    rdf     = _add_muon_columns(rdf=rdf)
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
    arr_ind     = numexpr.evaluate(q2_cond, local_dict={'q2' : jpsi_m * jpsi_m})

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
    d_mva_kind = _get_mva_config()

    d_mva_score = { f'mva_{name}' : _scores_from_rdf(rdf, d_path) for name, d_path in d_mva_kind.items() }

    d_data      = rdf.AsNumpy(['RUNNUMBER', 'EVENTNUMBER'])
    d_data.update(d_mva_score)
    rdf         = RDF.FromNumpy(d_data)

    return rdf
#---------------------------------
def _get_mva_config() -> dict:
    d_path_cmb = { q2bin : f'{Data.ana_dir}/mva/cmb/{Data.version}/{q2bin}' for q2bin in ['low', 'central', 'high'] }
    d_path_prc = { q2bin : f'{Data.ana_dir}/mva/prc/{Data.version}/{q2bin}' for q2bin in ['low', 'central', 'high'] }

    return {'cmb' : d_path_cmb, 'prc' : d_path_prc}
#---------------------------------
def _get_out_path(input_path : str) -> str:
    out_dir = f'{Data.ana_dir}/Data/mva/{Data.version}'
    name    = os.path.basename(input_path)

    os.makedirs(out_dir, exist_ok=True)

    return f'{out_dir}/{name}'
#---------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    _set_loggers()

    log.info('Getting dataframe')
    with RDFGetter.exclude_friends(names=['mva']):
        gtr   = RDFGetter(sample=Data.sample, trigger=Data.trigger)
        d_rdf = gtr.get_rdf(per_file=True)

    for inp_path, rdf in d_rdf.items():
        rdf      = _filter_rdf(rdf)
        out_path = _get_out_path(inp_path)

        if os.path.isfile(out_path):
            log.info('Output already found, skipping')
            return

        log.info(f'Producing: {out_path}')
        nentries = rdf.Count().GetValue()
        if nentries == 0:
            log.warning('Input datset is empty, saving empty dataframe')
            rdf = RDataFrame(0)
            rdf = rdf.Define('fake', '1')
            rdf.Snapshot('DecayTree', out_path)
            continue

        log.info('Applying classifier')
        rdf = _apply_classifier(rdf)

        if not Data.dry_run:
            log.info(f'Saving to: {out_path}')
            rdf.Snapshot('DecayTree', out_path)

    log.info('')
#---------------------------------
if __name__ == '__main__':
    main()
