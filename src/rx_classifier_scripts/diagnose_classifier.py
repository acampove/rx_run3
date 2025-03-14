'''
Script with code used to make diagnostics plots for classifiers
'''
import argparse

from dataclasses           import dataclass
from dmu.ml.cv_diagnostics import CVDiagnostics
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_classifier:diagnose_classifier')
#---------------------------------
@dataclass
class Data:
    '''
    Class used to store shared information
    '''
    max_path    = 700
    sample      : str
    trigger     : str
    cfg         : dict
    max_entries : int
    l_model     : list
    log_level   : int
    dry_run     : bool
#---------------------------------
def _get_args():
    '''
    Use argparser to put options in Data class
    '''
    parser = argparse.ArgumentParser(description='Script used to run diagnostic checks for classifier models')
    parser.add_argument('-c', '--conf'       , type=str, help='Version of config file'                      , required=True)
    parser.add_argument('-s', '--sample'     , type=str, help='Sample name, meant to exist inside input_dir', required=True)
    parser.add_argument('-t', '--trigger'    , type=str, help='HLT trigger'                                 , required=True)
    parser.add_argument('-l', '--log_level'  , type=int, help='Logging level', default=20, choices=[10, 20, 30])
    parser.add_argument('-m', '--max_entries', type=int, help='Limit datasets entries to this value', default=-1)
    parser.add_argument('-d', '--dry_run'    ,           help='Dry run', action='store_true')
    args = parser.parse_args()

    Data.sample      = args.sample
    Data.trigger     = args.trigger
    Data.cfg_path    = args.cfg_path
    Data.max_entries = args.max_entries
    Data.log_level   = args.log_level
    Data.dry_run     = args.dry_run
# -------------------------------
def main():
    '''
    Start here
    '''
    _load_config()
    l_model = _get_model()
    rdf     = _get_rdf()

    cvd = CVDiagnostics(models=l_model, rdf=rdf, cfg=Data.cfg['diagnostics'])
    cvd.run()
# -------------------------------
if __name__ == '__main__':
    main()
