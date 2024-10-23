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
    logl : int
    dry  : bool
    prnt : bool
    cfg  : dict
    l_ad : list[str]
    l_rm : list[str]
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

    log.debug(f'Running at {Data.logl} logging level')
#----------------------------
def _get_args():
    '''
    Will parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to edit and print server list specified by ~/.config/connect/servers.yaml')
    parser.add_argument('-p', '--print'  , help ='Prints config settings and exits', action='store_true')
    parser.add_argument('-l', '--log_lvl', type =int, help='Logging level', default=20, choices=[10,20,30])
    parser.add_argument('-a', '--add'    , nargs=3  , help='Adds task to given server, e.g. task 123 server'     , default=[])
    parser.add_argument('-r', '--rem'    , nargs=3  , help='Removes task from given server, e.g. task 123 server', default=[])
    parser.add_argument('-d', '--dry'    , help='Run dry run, for adding and removing entries', action='store_true')
    args   = parser.parse_args()

    Data.prnt = args.print
    Data.logl = args.log_lvl
    Data.l_ad = args.add
    Data.l_rm = args.rem
    Data.dry  = args.dry
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

    if Data.prnt:
        log.debug('Printing and returning')
        _print_configs()
        return

    cfg = _get_updated_config()
    _dump_config(cfg)
#---------------------------------------
if __name__ == '__main__':
    main()
