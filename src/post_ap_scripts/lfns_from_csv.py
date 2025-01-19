'''
Script that will make a list of dirac job IDs into a
text file with LFNs
'''
import os
import argparse

from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('post_ap:lfns_from_csv')
# ----------------------------
class Data:
    '''
    Data storing shared attributes
    '''
    version  : str
    grid_dir = '/eos/lhcb/grid/user/lhcb/user/a/acampove'
# ----------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Will use apd to save a list of paths to ROOT files in EOS')
    parser.add_argument('-v', '--version' , type=str, help='Version of production')
    parser.add_argument('-l', '--loglevel', type=int, help='Controls logging level', choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.version = args.version
    LogStore.set_level('post_ap:lfns_from_csv', args.loglevel)
# ----------------------------
def _initialize() -> None:
    if not os.path.isdir(Data.grid_dir):
        raise FileNotFoundError(f'Missing grid directory: {Data.grid_dir}')

    log.debug(f'Looking into: {Data.grid_dir}')
# ----------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()
# ----------------------------
if __name__ == '__main__':
    main()

    #/2025_01/1023704/1023704148/
