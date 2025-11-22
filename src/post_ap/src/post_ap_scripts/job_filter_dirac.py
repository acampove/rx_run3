'''
Script which will create filtering DIRAC jobs and use them to submit
filtering jobs
'''

import os
import json
import argparse
from typing              import Union
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
    maxj        : int
    dset        : str
    conf        : str
    venv        : str
    mode        : str
    epat        : str
    user        : str
    runner_path : str
    conf_name   : str
    pfn_path    : str
    dryr        : bool
    test_job    : bool
    l_resu      : list[int]
# ---------------------------------------
def _get_inputs() -> list[str]:
    return [
            f'LFN:/lhcb/user/{Data.user[0]}/{Data.user}/run3/venv/{Data.venv}/dcheck.tar',
            Data.conf,
            Data.pfn_path,
    ]
# ---------------------------------------
def _skip_job(jobid : int) -> bool:
    if len(Data.l_resu) == 0:
        return False

    if jobid in Data.l_resu:
        return False

    return True
# ---------------------------------------
def _get_job(jobid : int) -> Union[Job,None]:
    if _skip_job(jobid):
        return None

    l_input = _get_inputs()

    j = Job()
    j.setCPUTime(36000)
    j.setDestination('LCG.CERN.cern')
    j.setExecutable(Data.runner_path, arguments=f'{Data.prod} {Data.samp} {Data.conf_name}.yaml {Data.njob} {jobid} {Data.epat} {Data.user}')
    j.setInputSandbox(l_input)
    j.setOutputData(['*.root'], outputPath=f'{Data.name}_{Data.samp}')
    j.setName(f'{Data.name}_{jobid:03}')

    return j
# ---------------------------------------
def _get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Used to send filtering jobs to the grid')
    parser.add_argument('-n', '--name' , type =str, help='Name of job in dirac'        , required=True)
    parser.add_argument('-p', '--prod' , type =str, help='Name of production'          , required=True)
    parser.add_argument('-s', '--samp' , type =str, help='Sample nickname'             , required=True)
    parser.add_argument('-c', '--conf' , type =str, help='Path to config file'         , required=True)
    parser.add_argument('-e', '--venv' , type =str, help='Index of virtual environment', required=True)
    parser.add_argument('-u', '--user' , type =str, help='User associated to venv'     , required=True)
    parser.add_argument('-M', '--maxj' , type =int, help='Maximum number of jobs'      , default =500)
    parser.add_argument('-m', '--mode' , type =str, help='Run locally or in the grid'  , required=True, choices=['local', 'wms'])
    parser.add_argument('-r', '--resu' , nargs='+', help='List of jobs to resubmit, if not passed, it will send everything', default=[])
    parser.add_argument('-t', '--test' ,            help='If use, will do only one job', action='store_true')
    parser.add_argument('-d', '--dryr' ,            help='Skips jobs submission'       , action='store_true')

    args = parser.parse_args()

    return args
# ---------------------------------------
def _check_config() -> None:
    if not os.path.isfile(Data.conf):
        raise FileNotFoundError(f'File not found: {Data.conf}')

    Data.conf_name = os.path.basename(Data.conf).replace('.yaml', '')
# ---------------------------------------
def _pfns_to_njob(d_pfn : dict[str, list[str]]) -> int:
    npfn = 0
    for l_pfn in d_pfn.values():
        npfn += len(l_pfn)

    return npfn if npfn < Data.maxj else Data.maxj
# ---------------------------------------
def _get_pfns_path() -> str:
    with open(Data.conf, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    reader    = PFNReader(cfg=cfg)
    d_pfn     = reader.get_pfns(production=Data.prod, nickname=Data.samp)
    Data.njob = _pfns_to_njob(d_pfn)

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
    Data.maxj    = args.maxj
    Data.venv    = args.venv
    Data.user    = args.user
    Data.mode    = args.mode
    Data.dryr    = args.dryr
    Data.epat    = os.environ['VENVS']
    Data.test_job= args.test
    Data.l_resu  = [ int(jobid) for jobid in args.resu ]
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
        if job is None:
            continue

        if not Data.dryr:
            dirac.submitJob(job, mode=Data.mode)

        if Data.test_job or Data.name.startswith('test'):
            log.warning('Running a single test job')
            break

        if Data.mode == 'local':
            log.warning('Running a single local job')
            break
# ---------------------------------------
if __name__ == '__main__':
    main()
