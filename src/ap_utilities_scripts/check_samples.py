'''
Script used to check which MC samples are found in grid
'''
import argparse

from dataclasses                          import dataclass
from dmu.logging.log_store  import LogStore

from ap_utilities.bookkeeping.bkk_checker import BkkChecker

log=LogStore.add_logger('ap_utilities_scripts:check_samples')
# --------------------------------
@dataclass
class Data:
    '''
    Class storing shared attributes
    '''

    input_path  : str
    nthread     : int
    log_lvl     : int
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to filter samples based on what exists in the GRID')
    parser.add_argument('-i', '--input'  , type=str, help='Path to input YAML file')
    parser.add_argument('-n', '--nthread', type=int, help='Number of threads', default=1)
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10,20,30,40])
    args = parser.parse_args()

    Data.input_path  = args.input
    Data.nthread     = args.nthread
    Data.log_lvl     = args.log_lvl
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
    _set_logs()

    obj=BkkChecker(Data.input_path)
    obj.save(nthreads=Data.nthread)
# --------------------------------
if __name__ == '__main__':
    main()
