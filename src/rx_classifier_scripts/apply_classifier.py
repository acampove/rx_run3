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
    version     : str
    process_all : int
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
    parser.add_argument('-a', '--process_all', type=int, help='If 1 (default), will process all, if 0 will skip candidates that fail selection', default=1, choices=[0, 1])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    parser.add_argument('-d', '--dry_run'    ,           help='Dry run', action='store_true')
    parser.add_argument('-f', '--force_new'  ,           help='Will remake outputs, even if they already exist', action='store_true')
    args = parser.parse_args()

    Data.version     = args.version
    Data.sample      = args.sample
    Data.trigger     = args.trigger
    Data.log_level   = args.log_level
    Data.process_all = args.process_all
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
def _add_columns(rdf : RDataFrame) -> RDataFrame:
    if Data.process_all:
        log.info('Processing entire dataframe')
        return rdf

    log.info('Processing only entries that pass selection, except [q2, btd, pid_l, mass]')
    d_sel = sel.selection(
            trigger=Data.trigger,
            q2bin  = 'jpsi', # Does not matter, will remove q2 cut
            process=Data.sample)

    log.info('Adding prediction skipping column')

    # We need MVA scores only for the candidates passing full selection
    # except for the cuts below
    del d_sel['q2']
    del d_sel['bdt']
    del d_sel['pid_l'] # Need to remove the PID on electrons to allow MisID lines
    del d_sel['mass']

    l_expr   = list(d_sel.values())
    l_expr   = [ f'({expr})' for expr in l_expr ]
    full_cut = ' && '.join(l_expr)
    rdf      = rdf.Define('skip_mva_prediction', f'({full_cut}) == 0')

    return rdf
#---------------------------------
def _set_loggers():
    LogStore.set_level('dmu:ml:cv_predict'             , Data.log_level)
    LogStore.set_level('rx_classifier:apply_classifier', Data.log_level)
    LogStore.set_level('rx_data:rdf_getter'            , Data.log_level)
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

    if 'MuMu' in Data.trigger:
        log.info(f'Defining muon columns before prediction for trigger: {Data.trigger}')
        rdf = cut.add_muon_columns(rdf=rdf)
    else:
        log.info(f'Not defining muon columns for trigger: {Data.trigger}')

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
def _run(inp_path : str, rdf : RDataFrame) -> None:
    '''
    Takes ROOT dataframe and path associated to file

    Runs preliminary checks with early returns
    Calls prediction and saves file
    '''
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
        return

    log.info('Applying classifier')
    rdf = _apply_classifier(rdf)

    if not Data.dry_run:
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
        rdf = _filter_rdf(rdf)
        rdf = _add_columns(rdf)
        _run(inp_path=inp_path, rdf=rdf)
#---------------------------------
if __name__ == '__main__':
    main()
