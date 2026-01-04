'''
Script meant to steer calculation of friend trees
based on config file
'''

import os
import argparse
from omegaconf import DictConfig

from ihep_utilities        import JobSubmitter
from dmu.generic           import utilities as gut
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
    project  = ''
    wildcard = '*' # by default do all files
    exclude  = []
    only     = None
    cfg      : DictConfig
    dry_run  = False
    log_lvl  = 20
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script meant to steer calculation of friend trees in HTCondor at IHEP')
    parser.add_argument('-c', '--config' , type=str , help='Name of config file, e.g. rk_nopid', required=True)
    parser.add_argument('-p', '--project', type=str , help='Name of project, e.g. rk', required=True)
    parser.add_argument('-e', '--exclude', nargs='+', help='List of names of friend trees to exclude', default=[])
    parser.add_argument('-o', '--only'   , type=str , help='Name of the only friend tree to make')
    parser.add_argument('-w', '--wcard'  , type=str , help='Wildcard to match files', default=Data.wildcard)
    parser.add_argument('-d', '--dry_run', action='store_true', help='If used, it will do a dry run, fewer jobs and no outputs')
    parser.add_argument('-l', '--log_lvl', type=int , help='Logging level' , choices=[10, 20, 30], default=20)
    args = parser.parse_args()

    Data.cfg_name = args.config
    Data.project  = args.project
    Data.exclude  = args.exclude
    Data.only     = args.only
    Data.dry_run  = args.dry_run
    Data.wildcard = args.wcard
    Data.log_lvl  = args.log_lvl
# ----------------------
def _initialize() -> None:
    '''
    Initializes the configuration, etc
    '''
    Data.cfg = gut.load_conf(package='rx_data_data', fpath=f'friend_trees/{Data.cfg_name}.yaml')

    os.makedirs(Data.tmp_path, exist_ok=True)
    LogStore.set_level('rx_data:make_friend_trees', Data.log_lvl)
# ----------------------
def _get_commands(kind : str) -> list[str]:
    '''
    Parameters
    -------------
    kind : Kind of tree, e.g. hop, mva

    Returns
    -------------
    List of strings, each symbolizing a branch making process
    '''
    version = Data.cfg.versions[kind]
    npart   = Data.cfg.test_split[kind] if Data.dry_run else Data.cfg.splitting[kind]

    l_line = []
    for part in range(npart):
        line = f'branch_calculator -k {kind} -P {Data.project} -v {version} -w "{Data.wildcard}" -p {part} {npart} -b'
        if Data.dry_run:
            line = line + ' -d'

        l_line.append(line)

    return l_line
# ----------------------
def _skip_kind(kind : str) -> bool:
    '''
    Parameters
    -------------
    kind: Type of job, e.g. mva, hop

    Returns
    -------------
    True if this is meant to be skipped
    '''
    if kind in Data.exclude:
        return True

    if Data.only is None:
        return False

    if kind != Data.only:
        return True

    return False
# ----------------------
def _get_job_dictionary() -> dict[str,list[str]]:
    '''
    Returns
    -------------
    Dictionary where:
        Key  : Job identifier, e.g. mva, hop
        Value: List of commands to run for given job
    '''
    d_job = {}
    log.debug(40 * '-')
    log.debug(f'{"Job Type":<20}{"Number of jobs"}')
    log.debug(40 * '-')
    for kind in Data.cfg.versions:
        if _skip_kind(kind=kind):
            log.info(f'Excluding tree: {kind}')
            continue

        l_command   = _get_commands(kind=kind)
        ncommand    = len(l_command)

        log.debug(f'{kind:<20}{ncommand}')

        d_job[kind] = l_command

    njob=len(d_job)
    log.info(f'Built {njob} job lists')

    return d_job
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _initialize()
    d_job = _get_job_dictionary()
    sbt   = JobSubmitter(jobs=d_job, environment=Data.cfg.environment)
    sbt.run(skip_submit=Data.dry_run)
# ----------------------
if __name__ == '__main__':
    main()
