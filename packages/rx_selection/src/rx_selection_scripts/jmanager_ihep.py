'''
Script used to manage jobs in IHEP
'''

import argparse
import subprocess
import re
import os
from dataclasses import dataclass

from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_selection:jmanager_ihep')
#------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    user       = os.environ['USER']
    condor_cmd = ['hep_q', '-u', user]

    test       : int
    memory     : int
    l_held_job : list[str]
#------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to perform several operations on held jobs')
    parser.add_argument('-m', '--memory', type=int, help='Memory, in MB')
    parser.add_argument('-t', '--test'  , type=int, help='Testing flag', choices=[0, 1], default=1)
    parser.add_argument('-l', '--level' , type=int, help='Log level'   , choices=[10, 20, 30, 40], default=20)
    args = parser.parse_args()

    Data.memory = args.memory
    Data.test   = args.test

    log.setLevel(args.level)
#------------------------------
def _get_held_jobs() -> list[str]:
    output = subprocess.check_output(Data.condor_cmd, text=True)
    l_line = output.splitlines()

    l_job_id = []
    for line in l_line[1:]:
        line   = re.sub(' +', ' ', line)
        l_word = line.split(' ')
        if len(l_word) < 5 or l_word[5] != 'H':
            continue

        l_job_id.append(l_word[0])

    if len(l_job_id) == 0:
        log.error('No held jobs found in:')
        print(output)
        raise ValueError(f'Used command: {Data.condor_cmd}')

    return l_job_id
#------------------------------
def _run_command(cmd, options) -> None:
    if   Data.test == 1:
        log.info(f'{cmd}, {options}')
        return

    if Data.test == 0:
        log.debug(f'{cmd}, {options}')
        stat = subprocess.run([cmd] + options)
    else:
        raise ValueError(f'Invalid test flag: {Data.test}')

    if stat.returncode != 0:
        raise ValueError(f'Process returned exit status: {stat.returncode}')
#------------------------------
def memory():
    '''
    Will change jobs memory
    '''
    for job_id in Data.l_held_job:
        _run_command('hep_edit',  [job_id, '-m', str(Data.memory)])
#------------------------------
def _release_jobs():
    for job_id in Data.l_held_job:
        _run_command('hep_release',  [job_id])
#------------------------------
def main():
    '''
    Script starts here
    '''
    _get_args()
    Data.l_held_job = _get_held_jobs()

    if Data.memory is not None:
        log.debug('Doing memory edit')
        memory()
    else:
        raise ValueError('Not doing memory edit')

    _release_jobs()
#------------------------------
if __name__ == '__main__':
    main()
