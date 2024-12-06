'''
Script which will create filtering DIRAC jobs and use them to submit
filtering jobs
'''

import os
import json
import argparse
from importlib.resources import files

import yaml
from DIRAC.Interfaces.API.Dirac import Dirac
from DIRAC.Interfaces.API.Job   import Job
from DIRAC                      import initialize as initialize_dirac

from tqdm                  import trange
from dmu.logging.log_store import LogStore

from post_ap.pfn_reader    import PFNReader

log = LogStore.add_logger('post_ap:job_filter')
# ---------------------------------------
class Data:
    '''
    Class used to hold shared attributes
    '''
    name        : str
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
    pfn_path    : str
    test_job    : bool
# ---------------------------------------
def _get_inputs() -> list[str]:
    return [
            f'LFN:/lhcb/user/{Data.user[0]}/{Data.user}/run3/venv/{Data.venv}/dcheck.tar',
            Data.conf,
            Data.pfn_path,
    ]
# ---------------------------------------
def _get_job(jobid : int) -> Job:
    l_input = _get_inputs()

    j = Job()
    j.setCPUTime(36000)
    j.setDestination('LCG.CERN.cern')
    j.setExecutable(Data.runner_path, arguments=f'{Data.prod} {Data.samp} {Data.conf_name}.yaml {Data.njob} {jobid} {Data.epat} {Data.user}')
    j.setInputSandbox(l_input)
    j.setOutputData(['*.root'], outputPath=f'{Data.name}_{Data.samp}')
    j.setName(Data.name)

    return j
# ---------------------------------------
def _get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Used to send filtering jobs to the grid')
    parser.add_argument('-n', '--name' , type =str, help='Name of job in dirac'        , required=True)
    parser.add_argument('-p', '--prod' , type =str, help='Name of production'          , required=True)
    parser.add_argument('-s', '--samp' , type =str, help='Sample nickname'             , required=True)
    parser.add_argument('-c', '--conf' , type =str, help='Path to config file'         , required=True)
    parser.add_argument('-j', '--njob' , type =int, help='Number of grid jobs'         , required=True)
    parser.add_argument('-e', '--venv' , type =str, help='Index of virtual environment', required=True)
    parser.add_argument('-u', '--user' , type =str, help='User associated to venv'     , required=True)
    parser.add_argument('-m', '--mode' , type =str, help='Run locally or in the grid'  , required=True, choices=['local', 'wms'])
    parser.add_argument('-t', '--test' ,            help='If use, will do only one job', action='store_true')

    args = parser.parse_args()

    return args
# ---------------------------------------
def _check_config() -> None:
    if not os.path.isfile(Data.conf):
        raise FileNotFoundError(f'File not found: {Data.conf}')

    Data.conf_name = os.path.basename(Data.conf).replace('.yaml', '')
# ---------------------------------------
def _get_pfns_path() -> str:
    with open(Data.conf, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    reader = PFNReader(cfg=cfg)
    d_pfn  = reader.get_pfns(production=Data.prod, nickname=Data.samp)

    ofile_path = '/tmp/pfns.json'
    with open(ofile_path, 'w', encoding='utf-8') as ofile:
        json.dump(d_pfn, ofile, indent=4)

    return ofile_path
# ---------------------------------------
def _initialize() -> None:
    args         = _get_args()
    Data.name    = args.name
    Data.prod    = args.prod
    Data.samp    = args.samp
    Data.conf    = args.conf
    Data.njob    = args.njob
    Data.venv    = args.venv
    Data.user    = args.user
    Data.mode    = args.mode
    Data.epat    = os.environ['VENVS']
    Data.test_job= args.test
    Data.pfn_path= _get_pfns_path()

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

        if Data.test_job:
            log.warning('Running a single test job')
            break

        if Data.mode == 'local':
            log.warning('Running a single local job')
            break
# ---------------------------------------
if __name__ == '__main__':
    main()
