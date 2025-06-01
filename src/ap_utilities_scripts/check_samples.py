'''
Script used to check which MC samples are found in grid
'''
import argparse

from importlib.resources                  import files
from dataclasses                          import dataclass

import yaml
from ap_utilities.logging.log_store       import LogStore
from ap_utilities.bookkeeping.bkk_checker import BkkChecker

log=LogStore.add_logger('ap_utilities_scripts:check_samples')
# --------------------------------
@dataclass
class Data:
    '''
    Class storing shared attributes
    '''
    config  : str
    nthread : int
    log_lvl : int

    config_path : str
# ----------------------------------------
def _initialize() -> None:
    config_path      = files('ap_utilities_data').joinpath(f'{Data.config}.yaml')
    Data.config_path = str(config_path)

    _set_logs()
# ----------------------------------------
def _sections_from_path(path : str) -> dict[str, dict]:
    with open(path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    return d_cfg
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to filter samples based on what exists in the GRID')
    parser.add_argument('-c', '--config' , type=str, help='Config file specifying samples to check')
    parser.add_argument('-n', '--nthread', type=int, help='Number of threads', default=1)
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10,20,30,40])
    args = parser.parse_args()

    Data.config   = args.config
    Data.nthread  = args.nthread
    Data.log_lvl  = args.log_lvl
# --------------------------------
def _set_logs() -> None:
    log.debug(f'Running with log level: {Data.log_lvl}')

    LogStore.set_level('ap_utilities:Bookkeeping.bkk_checker', Data.log_lvl)
    LogStore.set_level('ap_utilities_scripts:check_samples'  , Data.log_lvl)
# --------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    d_cfg      = _sections_from_path(Data.config_path)
    d_sections = d_cfg['sections']
    for name, d_section in d_sections.items():
        log.info(f'Processing section: {name}')
        obj=BkkChecker(name, d_section)
        obj.save(nthreads=Data.nthread)
# --------------------------------
if __name__ == '__main__':
    main()
