'''
Script in charge of comparing different classifiers' performances
'''
import argparse
from importlib.resources   import files

from dmu.ml.cv_performance import CVPerformance
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities     as gut

log=LogStore.add_logger('rx_classifier:compare_classifier')
# -------------------------------
class Data:
    '''
    Data class
    '''
    cfg : dict
# -------------------------------
def _load_config(name : str) -> dict:
    fpath = files('rx_classifier_data').joinpath(f'performance/{name}.yaml')
    fpath = str(fpath)
    data  = gut.load_yaml(fpath)

    return data
# -------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to compare performance of classifiers')
    parser.add_argument('-c', '--config', type=str, help='Name of config file', required=True)
    args = parser.parse_args()

    Data.cfg = _load_config(name=args.config)
# -------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    per = CVPerformance()
# -------------------------------
if __name__ == '__main__':
    main()
