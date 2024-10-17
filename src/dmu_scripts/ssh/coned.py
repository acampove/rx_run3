'''
Script used to implement connection to servers
'''

import os
import argparse

import yaml
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:scripts:coned')
#---------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    prnt : bool
    srvr : str
    logl : int
    cfg  : dict
#----------------------------
def _print_configs():
    '''
    Prints configuration
    '''

    yaml_output = yaml.dump(Data.cfg, default_flow_style=False)
    print(yaml_output)
#----------------------------
def _initialize():
    _load_config()

    LogStore.set_level('dmu:scripts:coned', Data.logl)
#----------------------------
def _get_args():
    '''
    Will parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to edit and print server list specified by ~/.config/connect/servers.yaml')
    parser.add_argument('-l', '--log_lvl', type=int, help='Logging level', default=20, choices=[10,20,30])
    parser.add_argument('-p', '--print'  , help='Prints config settings and exits', action='store_true')
    args   = parser.parse_args()

    Data.prnt = args.print
    Data.logl = args.log_lvl
#---------------------------------------
def _load_config():
    home_dir    = os.environ['HOME']
    config_path = f'{home_dir}/.config/dmu/ssh/servers.yaml'
    if not os.path.isfile(config_path):
        raise FileNotFoundError(f'Config not found: {config_path}')

    with open(config_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)
#---------------------------------------
def main():
    '''
    Starts here
    '''
    _get_args()
    _initialize()

    if Data.prnt is not None:
        _print_configs()
        return
#---------------------------------------
if __name__ == '__main__':
    main()
