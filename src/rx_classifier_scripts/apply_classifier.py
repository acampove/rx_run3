'''
Script used to create files with trimmed (few branches) trees containing classifier score
'''

import os
import glob
import argparse

from dataclasses         import dataclass
import joblib
import numpy

from ROOT                  import RDataFrame, RDF
from dmu.ml.cv_predict     import CVPredict
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel
from rx_classifier         import utilities as cut

log = LogStore.add_logger('rx_classifier:apply_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class used to store shared information
    '''
    max_path    = 700
    ana_dir     = os.environ['ANADIR']
    default_q2  = 'central' # Any entry not in [low, central, high] bins will go to this bin for prediction
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
def _range_rdf(rdf : RDataFrame) -> RDataFrame:
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
    if 'MuMu' not in Data.trigger:
        log.info(f'Not defining muon columns for trigger: {Data.trigger}')
        return rdf

    log.info(f'Defining muon columns before prediction for trigger: {Data.trigger}')
    # These are the brem_track_2 columns, which only make sense for the electrons
    # For muons, they should be the same as the original columns
    rdf = cut.add_muon_columns(rdf=rdf)

    return rdf
#---------------------------------
def _add_common_columns(rdf : RDataFrame) -> RDataFrame:
    '''
    Adds columns needed by all samples
    '''
    # Needed to align scores with entries
    rdf = rdf.Define('index', 'rdfentry_')

    return rdf
#---------------------------------
def _set_loggers():
    LogStore.set_level('rx_classifier:apply_classifier', Data.log_level)

    # Dependencies in custom level, if custom level is debug
    # Otherwise keep them in WARNING
    if Data.log_level < 20:
        LogStore.set_level('rx_classifier:utilities', Data.log_level)
        LogStore.set_level('rx_selection:selection' , Data.log_level)
        LogStore.set_level('rx_data:rdf_getter'     , Data.log_level)
        LogStore.set_level('rx_data:path_splitter'  , Data.log_level)
        LogStore.set_level('dmu:ml:cv_predict'      , Data.log_level)
    else:
        LogStore.set_level('rx_classifier:utilities',             30)
        LogStore.set_level('rx_selection:selection' ,             30)
        LogStore.set_level('rx_data:rdf_getter'     ,             30)
        LogStore.set_level('rx_data:path_splitter'  ,             30)
        LogStore.set_level('dmu:ml:cv_predict'      ,             30)
#---------------------------------
def _get_q2_selection(q2bin : str) -> str:
    d_sel = sel.selection(
            trigger=Data.trigger,
            q2bin  =q2bin,
            process=Data.sample)

    q2_cut = d_sel['q2']

    return q2_cut
#---------------------------------
def _apply_q2_cut(
        rdf   : RDataFrame,
        q2bin : str) -> RDataFrame:
    '''
    Applies q2 requirement to ROOT dataframe
    '''
    if q2bin == 'rest':
        low     = _get_q2_selection(q2bin='low')
        central = _get_q2_selection(q2bin='central')
        high    = _get_q2_selection(q2bin='high')
        q2_cut  = f'!({low}) && !({central}) && !({high})'
    else:
        q2_cut  = _get_q2_selection(q2bin=q2bin)

    log.debug(f'{q2bin:<10}{q2_cut}')
    rdf = rdf.Filter(q2_cut, 'q2')

    return rdf
# ----------------------------------------
def _q2_scores_from_rdf(
        rdf    : RDataFrame,
        d_path : dict[str,str],
        q2bin  : str) -> numpy.ndarray:
    '''
    Parameters
    -----------
    rdf   : DataFrame with input data, it has to be indexed with an `index` column
    d_path: Dictionary mapping q2bin to path to models
    q2bin : q2 bin

    Returns
    -----------
    2D Array with indexes and MVA scores
    '''
    rdf     = _apply_q2_cut(rdf=rdf, q2bin=q2bin)
    nentries= rdf.Count().GetValue()
    if nentries == 0:
        log.warning(f'No entries found for q2 bin: {q2bin}')
        return numpy.column_stack(([], []))

    # The dataframe has the correct cut applied
    # From here onwards, if the q2bin is non-rare (rest)
    # Will use default_q2 model
    if q2bin == 'rest':
        q2bin = Data.default_q2

    path   = d_path[q2bin]
    l_pkl  = glob.glob(f'{path}/*.pkl')

    npkl   = len(l_pkl)
    if npkl == 0:
        raise ValueError(f'No pickle files found in {path}')

    log.info(f'Using {npkl} pickle files from: {path}')
    l_model = [ joblib.load(pkl_path) for pkl_path in l_pkl ]


    cvp     = CVPredict(models=l_model, rdf=rdf)
    if Data.dry_run:
        log.warning(f'Using {nentries} ones for dry run MVA scores')
        arr_prb = numpy.ones(nentries)
    else:
        arr_prb = cvp.predict()

    arr_ind = rdf.AsNumpy(['index'])['index']
    arr_res = numpy.column_stack((arr_ind, arr_prb))

    log.debug(f'Shape: {arr_res.shape}')

    return arr_res
# ----------------------------------------
def _scores_from_rdf(
        rdf    : RDataFrame,
        d_path : dict[str,str]) -> numpy.ndarray:
    '''
    Parameters
    ------------------
    rdf   : DataFrame with inputs
    d_path: Dictionary mapping q2bin to path to models

    Returns
    ------------------
    Array of signal probabilities
    '''
    nentries = rdf.Count().GetValue()

    arr_low     = _q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='low'    )
    arr_central = _q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='central')
    arr_high    = _q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='high'   )
    arr_rest    = _q2_scores_from_rdf(rdf=rdf, d_path=d_path, q2bin='rest'   )
    arr_all     = numpy.concatenate((arr_low, arr_central, arr_high, arr_rest))

    arr_ind = arr_all.T[0]
    arr_val = arr_all.T[1]

    arr_obtained = numpy.sort(arr_ind)
    arr_expected = numpy.arange(nentries + 1)
    if  numpy.array_equal(arr_obtained, arr_expected):
        raise ValueError('Array of indexes has the wrong values')

    arr_ord = numpy.argsort(arr_ind)
    arr_mva = arr_val[arr_ord]

    return arr_mva
# ----------------------------------------
def _apply_classifier(rdf : RDataFrame) -> RDataFrame:
    '''
    Takes name of dataset and corresponding ROOT dataframe
    return dataframe with a classifier probability column added
    '''
    d_mva_kind  = _get_mva_config()
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
def _skip_rdf(inp_path : str, rdf : RDataFrame) -> bool:
    out_path = _get_out_path(inp_path)
    if os.path.isfile(out_path):
        log.info('Output already found, skipping')
        return True

    log.info(f'Producing: {out_path}')
    nentries = rdf.Count().GetValue()
    if nentries == 0:
        log.warning('Input datset is empty, saving empty dataframe')
        rdf = RDataFrame(0)
        rdf = rdf.Define('fake', '1')
        rdf.Snapshot('DecayTree', out_path)
        return True

    return False
#---------------------------------
def _run(inp_path : str, rdf : RDataFrame) -> None:
    '''
    Takes ROOT dataframe and path associated to file

    Runs preliminary checks with early returns
    Calls prediction and saves file
    '''

    log.info('Applying classifier')
    rdf = _apply_classifier(rdf)

    if Data.dry_run:
        return

    out_path = _get_out_path(inp_path)
    log.info(f'Saving to: {out_path}')
    rdf.Snapshot('DecayTree', out_path)
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
        if _skip_rdf(inp_path=inp_path, rdf=rdf):
            continue

        rdf = _range_rdf(rdf)
        rdf = _add_muon_columns(rdf)
        rdf = _add_common_columns(rdf)
        _run(inp_path=inp_path, rdf=rdf)
#---------------------------------
if __name__ == '__main__':
    main()
