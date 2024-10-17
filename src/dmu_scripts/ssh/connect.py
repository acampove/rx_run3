'''
Script used to implement connection to servers
'''

import os
import pprint
import argparse
import subprocess

import yaml
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:scripts:connect')
#---------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    prnt : str
    name : str
    cfg  : dict
#----------------------------
def _print_configs():
    '''
    Prints configuration
    '''

    if Data.prnt == 'servers':
        log.info('Severs:')
        for server in Data.cfg:
            log.info(server)

    else:
        raise ValueError(f'Invalid print quantity: {Data.prnt}')
#----------------------------
def _run_command(cmd : str, options : list , raise_on_fail : bool) -> None:
    '''
    Runs command

    cmd            (str): Command, i.e. ssh
    options       (list): List of options, e.g. [-X, user@host]
    raise_on_fail (bool): If true will end process if command does not return 0
    '''
    log.debug('-' * 30)
    log.debug('-' * 30)
    log.debug(f'{cmd:<10}{str(options):<50}')
    log.debug('-' * 30)
    log.debug('-' * 30)

    with open('/tmp/rk_ext_output.log', 'w', encoding='utf-8') as ofile:
        stat = subprocess.run([cmd] + options, stdout=ofile, stderr=ofile, check=False)

    if stat.returncode != 0 and raise_on_fail:
        log.error(f'Failed: {cmd:<10}{str(options):<50}')
        raise ValueError(f'Process returned exit status: {stat.returncode}')
#---------------------------------------
def _get_args():
    '''
    Will parse arguments
    '''
    parser = argparse.ArgumentParser(description='Used to connect through SSH to servers specified by ~/.config/connect/servers.yaml')
    parser.add_argument('-n', '--name'  , type=str, help='Name of task'                                      , required=True)
    parser.add_argument('-s', '--server', type=str, help='Substring that will uniquely determine server name', required=True)
    parser.add_argument('-p', '--print' , type=str, help='Prints config settings and exits')
    args   = parser.parse_args()

    Data.name = args.name
    Data.prnt = args.print
    Data.srvr = args.server
#---------------------------------------
def _load_config():
    home_dir    = os.environ['HOME']
    config_path = f'{home_dir}/.config/dmu/ssh/servers.yaml'
    if not os.path.isfile(config_path):
        raise FileNotFoundError(f'Config not found: {config_path}')

    with open(config_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)
#---------------------------------------
def _get_options() -> list:
    '''
    Will return options for SSH
    '''

    return []
#---------------------------------------
def _connect():
    '''
    Will connect through SSH
    '''

    options = _get_options()
    #_run_command(cmd = 'ssh', options = options, raise_on_fail=True)
#---------------------------------------
def main():
    '''
    Starts here
    '''
    _get_args()
    _load_config()

    if Data.prnt is not None:
        _print_configs()
        return

    _connect()
#---------------------------------------
if __name__ == '__main__':
    main()
