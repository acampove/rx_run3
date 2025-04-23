'''
Script used to check what samples are present in the main trees
but missing among the friend trees
'''
import os
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_data:check_missing')
# ---------------------------------
class Data:
    '''
    Data class
    '''
    data_dir : str
# ---------------------------------
def _initialize() -> None:
    if 'DATADIR' not in os.environ:
        raise FileNotFoundError('DATADIR environment variable not set')

    Data.data_dir = os.environ['DATADIR']
# ---------------------------------
def main():
    '''
    Start here
    '''
    _initialize()

# ---------------------------------
if __name__ == '__main__':
    main()
