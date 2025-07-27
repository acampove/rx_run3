'''
Script meant to steer calculation of friend trees
based on config file
'''

import os
import argparse
from omegaconf import DictConfig

from ihep_utilities        import JobSubmitter
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:make_friend_trees')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    user     = os.environ['USER']
    tmp_path = f'/tmp/{user}/rx_data/jobs/friend_trees'
    cfg_name = ''
    exclude  = []
    only     = None
    cfg      : DictConfig
    dry_run  = False
    log_lvl  = 20
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script meant to steer calculation of friend trees in HTCondor at IHEP')
    parser.add_argument('-c', '--config' , type=str , help='Name of config file, e.g. nopid', required=True)
    parser.add_argument('-e', '--exclude', nargs='+', help='List of names of friend trees to exclude', default=[])
    parser.add_argument('-o', '--only'   , type=str , help='Name the the only friend tree')
    parser.add_argument('-d', '--dry_run', action='store_true', help='If used, it will do a dry run, fewer jobs and no outputs')
    parser.add_argument('-l', '--log_lvl', type=int , help='Logging level' , choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.cfg_name = args.config
    Data.exclude  = args.exclude
    Data.only     = args.only
    Data.dry_run  = args.dry_run
    Data.log_lvl  = args.log_lvl
# ----------------------
def _initialize() -> None:
    '''
    Initializes the configuration, etc
    '''

# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _initialize()
    l_path = _make_submission_files()
    for path in l_path:
        sbt = JobSubmitter(path=path, environment=Data.cfg.environment)
        sbt.run()
# ----------------------
if __name__ == '__main__':
    main()
