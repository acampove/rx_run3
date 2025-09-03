'''
Script used to check which MC samples are found in grid
'''
import argparse

from dataclasses                          import dataclass
from ap_utilities.logging.log_store       import LogStore
from ap_utilities.bookkeeping.bkk_checker import BkkChecker
from ap_utilities.bookkeeping             import sample_config as scf 

log=LogStore.add_logger('ap_utilities_scripts:check_samples')
# --------------------------------
@dataclass
class Data:
    '''
    Class storing shared attributes
    '''
    samples : str
    config  : str
    nthread : int
    log_lvl : int
# ----------------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to filter samples based on what exists in the GRID')
    parser.add_argument('-s', '--samples', type=str, help='Name of file storing event types, e.g. by_priority', required=True)
    parser.add_argument('-c', '--config' , type=str, help='Name of file storing configuration, e.g. 2024', required=True)
    parser.add_argument('-n', '--nthread', type=int, help='Number of threads', default=1)
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10,20,30,40])
    args = parser.parse_args()

    Data.samples  = args.samples
    Data.config   = args.config
    Data.nthread  = args.nthread
    Data.log_lvl  = args.log_lvl
# --------------------------------
def _set_logs() -> None:
    log.debug(f'Running with log level: {Data.log_lvl}')

    LogStore.set_level('ap_utilities:bkk_checker'          , Data.log_lvl)
    LogStore.set_level('ap_utilities:sample_config'        , Data.log_lvl)
    LogStore.set_level('ap_utilities_scripts:check_samples', Data.log_lvl)
# --------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _set_logs()

    obj = scf.SampleConfig(settings='2024', samples='by_priority')
    cfg = obj.get_config(categories=['high', 'medium', 'low'])

    for name, section in cfg.sections.items():
        log.info(f'Processing section: {name}')
        obj=BkkChecker(name, section)
        obj.save(nthreads=Data.nthread, dry=True)
# --------------------------------
if __name__ == '__main__':
    main()
