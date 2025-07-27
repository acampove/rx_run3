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
    os.makedirs(Data.tmp_path, exist_ok=True)
    LogStore.set_level('rx_data:make_friend_trees', Data.log_lvl)
# ----------------------
def _make_file_kind(kind : str) -> str:
    '''
    Parameters
    -------------
    kind : Kind of tree, e.g. hop, mva

    Returns
    -------------
    Path to text file where the commands go
    '''
    version = Data.cfg.versions[kind]
    npart   = Data.cfg.test_split[kind] if Data.dry_run else Data.cfg.splitting[kind]

    l_line = []
    for part in range(npart):
        line = f'branch_calculator -k {kind} -P {Data.cfg_name} -v {version} -p {part} {npart} -b'
        if Data.dry_run:
            line = line + ' -d'

        l_line.append(line)

    fpath = f'{Data.tmp_path}/{kind}.txt'
    with open(fpath, 'w', encoding='utf-8') as ofile:
        data = '\n'.join(l_line)
        ofile.write(data)

    log.debug(f'Writting: {fpath}')

    return fpath
# ----------------------
def _make_submission_files() -> list[str]:
    '''
    Using the configuration, build text files with commands to run in jobs

    Returns
    -------------
    List of paths to files with commands
    '''
    l_path = []
    for kind in Data.cfg.versions:
        path = _make_file_kind(kind=kind)
        l_path.append(path)

    npath=len(l_path)
    log.info(f'Built {npath} submission files')

    return l_path
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
