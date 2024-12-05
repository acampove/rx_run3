'''
Script which will create filtering DIRAC jobs and use them to submit
filtering jobs
'''

import os
import argparse
from importlib.resources import files

from DIRAC.Interfaces.API.Dirac import Dirac
from DIRAC.Interfaces.API.Job   import Job
from DIRAC                      import initialize as initialize_dirac

from tqdm    import trange

# ---------------------------------------
class Data:
    '''
    Class used to hold shared attributes
    '''
    prod        : str
    samp        : str
    njob        : int
    dset        : str
    conf        : str
    venv        : str
    mode        : str
    epat        : str
    user        : str
    runner_path : str
    conf_name   : str
# ---------------------------------------
def _get_inputs() -> list[str]:
    return [
            f'LFN:/lhcb/user/{Data.user[0]}/{Data.user}/run3/venv/{Data.venv}/dcheck.tar',
            Data.conf,
    ]
# ---------------------------------------
def _get_job(jobid : int) -> Job:
    l_input = _get_inputs()

    j = Job()
    j.setCPUTime(36000)
    j.setDestination('LCG.CERN.cern')
    j.setExecutable(Data.runner_path, arguments=f'{Data.prod} {Data.samp} {Data.conf_name}.yaml {Data.njob} {jobid} {Data.epat} {Data.user}')
    j.setInputSandbox(l_input)
    j.setOutputData(['*.root'], outputPath=f'{Data.conf_name}_{Data.samp}')
    j.setName(f'{Data.conf_name}_{Data.samp}_{jobid:03}')

    return j
# ---------------------------------------
def _get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Used to send filtering jobs to the grid')
    parser.add_argument('-p', '--prod' , type =str, help='Name of production'          , required=True)
    parser.add_argument('-s', '--samp' , type =str, help='Sample nickname'             , required=True)
    parser.add_argument('-c', '--conf' , type =str, help='Path to config file'         , required=True)
    parser.add_argument('-j', '--njob' , type =int, help='Number of grid jobs'         , required=True)
    parser.add_argument('-e', '--venv' , type =str, help='Index of virtual environment', required=True)
    parser.add_argument('-u', '--user' , type =str, help='User associated to venv'     , required=True)
    parser.add_argument('-m', '--mode' , type =str, help='Run locally or in the grid'  , required=True, choices=['local', 'wms'])

    args = parser.parse_args()

    return args
# ---------------------------------------
def _check_config() -> None:
    if not os.path.isfile(Data.conf):
        raise FileNotFoundError(f'File not found: {Data.conf}')

    Data.conf_name = os.path.basename(Data.conf).replace('.yaml', '')
# ---------------------------------------
def _initialize() -> None:
    args         = _get_args()
    Data.conf    = args.conf
    Data.njob    = args.njob
    Data.venv    = args.venv
    Data.user    = args.user
    Data.mode    = args.mode
    Data.epat    = os.environ['VENVS']

    _check_config()

    initialize_dirac()

    runner_path      = files('post_ap_grid').joinpath('run_filter')
    Data.runner_path = str(runner_path)
# ---------------------------------------
def main():
    '''
    Script starts here
    '''
    _initialize()

    dirac = Dirac()
    for jobid in trange(Data.njob, ascii=' -'):
        job    = _get_job(jobid)
        dirac.submitJob(job, mode=Data.mode)

        if 'test' in Data.conf_name:
            break

        if Data.mode == 'local':
            break
# ---------------------------------------
if __name__ == '__main__':
    main()
