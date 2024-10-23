'''
Script used to implement connection to servers
'''

import os
import copy
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
def _dump_config(cfg : dict):
    if cfg == Data.cfg:
        log.debug('Config was not modified, will not save it')
        return

    home_dir    = os.environ['HOME']
    config_path = f'{home_dir}/.config/dmu/ssh/servers.yaml'
    if not os.path.isfile(config_path):
        raise FileNotFoundError(f'Config not found: {config_path}')

    if Data.dry:
        content = yaml.dump(cfg, default_flow_style=False)
        print(content)
        return

    with open(config_path, 'w', encoding='utf-8') as ofile:
        yaml.dump(cfg, ofile, default_flow_style=False)
#---------------------------------------
def _get_updated_config() -> dict:
    log.debug('Getting updated config')

    cfg = copy.deepcopy(Data.cfg)
    cfg = _add_task(cfg)
    cfg = _remove_task(cfg)

    return cfg
#---------------------------------------
def _add_task(cfg : dict) -> dict:
    if len(Data.l_ad) == 0:
        log.debug('No task added')
        return cfg

    [task, machine, server] = Data.l_ad
    if server not in cfg:
        cfg[server] = {}

    if machine not in cfg[server]:
        cfg[server][machine] = []

    cfg[server][machine].append(task)

    log.info(f'{"Added":<10}{server:<20}{machine:<10}{task:<20}')

    return cfg
#---------------------------------------
def _remove_task(cfg : dict) -> dict:
    if len(Data.l_rm) == 0:
        log.debug('No task removed')
        return cfg

    [task, machine, server] = Data.l_rm
    if server not in cfg:
        log.warning(f'Server {server} not found')
        return cfg

    if machine not in cfg[server]:
        log.warning(f'Machine {machine} not found in server {server}')
        return cfg

    l_task = cfg[server][machine]
    if task not in l_task:
        log.warning(f'Task {task} not found in {server}:{machine}')
        return cfg

    index = l_task.index(task)
    del l_task[index]
    cfg[server][machine] = l_task

    log.info(f'{"Removed":<10}{server:<20}{machine:<10}{task:<20}')

    cfg = _trim_config(cfg, machine, server)

    return cfg
#---------------------------------------
def _trim_config(cfg : dict, machine : str, server : str) -> dict:
    if cfg[server][machine] == []:
        log.debug(f'Trimming {server}:{machine}')
        del cfg[server][machine]

    if cfg[server] == {}:
        log.debug(f'Trimming {server}')
        del cfg[server]

    return cfg
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
