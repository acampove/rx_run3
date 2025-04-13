'''
Script used to submit fitering jobs using Ganga
'''
# pylint: disable=line-too-long,too-many-instance-attributes, import-error

import os
import json
import argparse

from typing              import Union
from dataclasses         import dataclass
from importlib.resources import files

import yaml
import ganga.ganga
from ganga import Job, Executable, DiracFile, LocalFile, Interactive, GenericSplitter, Dirac, Local, File

from dmu.logging.log_store     import LogStore
from post_ap.pfn_reader        import PFNReader

log = LogStore.add_logger('post_ap:job_filter_ganga')
# -------------------------------------------------
@dataclass
class Data:
    '''
    Class used to share data
    '''
    name     : str
    prod     : str
    samp     : str
    conf     : str
    venv     : str
    back     : str
    env_lfn  : str
    logl     : int
    njob     : int
    test     : bool
    cfg      : dict
    dry_run  : bool

    maxj     = 500
    env_path = '/home/acampove/VENVS'
    user     = 'acampove'
    pfn_path = '/tmp/pfns.json'
    runner   = 'run_filter'
    platform = 'x86_64-el9-gcc11-opt'
    grid_site= 'LCG.CERN.cern'
# -------------------------------------------------
def _initialize() -> None:
    LogStore.set_level('post_ap:job_filter_ganga', Data.logl)
    LogStore.set_level('post_ap:pfn_reader'      , Data.logl)

    Data.env_lfn = f'LFN:/lhcb/user/{Data.user[0]}/{Data.user}/run3/venv/{Data.venv}/dcheck.tar'
    with open(Data.conf, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)

    _save_pfns()
# -------------------------------------------------
def _pfns_to_njob(d_pfn : dict[str, list[str]]) -> int:
    npfn = 0
    for l_pfn in d_pfn.values():
        npfn += len(l_pfn)

    return npfn if npfn < Data.maxj else Data.maxj
# -------------------------------------------------
def _save_pfns() -> None:
    reader    = PFNReader(cfg=Data.cfg)
    d_pfn     = reader.get_pfns(production=Data.prod, nickname=Data.samp)
    Data.njob = _pfns_to_njob(d_pfn)

    log.info(f'Will use {Data.njob} jobs')

    with open(Data.pfn_path, 'w', encoding='utf-8') as ofile:
        json.dump(d_pfn, ofile, indent=4)
# -------------------------------------------------
def _get_splitter() -> GenericSplitter:
    splitter           = GenericSplitter()
    splitter.attribute = 'application.args'
    splitter.values    = _get_splitter_args()

    return splitter
# -------------------------------------------------
def _get_splitter_args() -> list[list]:
    njob = Data.njob
    # Regardless of how many jobs were specified
    # Will run only one job if:
    #
    # 1. It runs locally/interactively
    # 2. The test flag was passed
    if Data.back in ['Interactive', 'Local'] or Data.test:
        log.warning('Using only one job')
        njob = 1

    conf = os.path.basename(Data.conf)

    return [ [Data.prod, Data.samp, conf, Data.njob, i_job, Data.env_path, Data.user] for i_job in range(njob) ]
# -------------------------------------------------
def _get_dirac_backend() -> Dirac:
    backend = Dirac()
    backend.settings['Destination'] = Data.grid_site

    return backend
# -------------------------------------------------
def _get_backend() -> Union[Dirac, Interactive]:
    if Data.back == 'Interactive':
        return Interactive()

    if Data.back == 'Local':
        return Local()

    if Data.back == 'Dirac':
        return _get_dirac_backend()

    raise ValueError(f'Invalid backend: {Data.back}')
# -------------------------------------------------
def _parse_args() -> None:
    parser=argparse.ArgumentParser(description='Script used to send ntuple filtering jobs to the Grid, through ganga')
    parser.add_argument('-n' , '--name', type=str, help='Job name'           , required=True)
    parser.add_argument('-p' , '--prod', type=str, help='Production'         , required=True)
    parser.add_argument('-s' , '--samp', type=str, help='Sample'             , required=True)
    parser.add_argument('-f' , '--path', type=str, help='Path to config file, proficed by user')
    parser.add_argument('-c' , '--conf', type=str, help='Version of config file belonging to project itself')
    parser.add_argument('-b' , '--back', type=str, help='Backend'            , choices=['Interactive', 'Local', 'Dirac'], default='Interactive')
    parser.add_argument('-t' , '--test',           help='Will run one job only if used'                       , action='store_true')
    parser.add_argument('-v' , '--venv', type=str, help='Version of virtual environment used to run filtering', required=True)
    parser.add_argument('-l' , '--logl', type=int, help='Logging level', choices=[10,20,30], default=10)
    parser.add_argument('-d' , '--dry_run',        help='If used, will not create and send job, only initialize', action='store_true')
    args = parser.parse_args()

    Data.name = args.name
    Data.prod = args.prod
    Data.samp = args.samp
    Data.back = args.back
    Data.venv = args.venv
    Data.test = args.test
    Data.logl = args.logl

    Data.dry_run = args.dry_run
    Data.conf    = _get_conf_path(args)
# -------------------------------------------------
def _get_conf_path(args : argparse.Namespace) -> str:
    '''
    Returns path to yaml config file from argparser
    '''
    if args.path is     None and args.conf is     None:
        raise ValueError('Neither path to config, nor version were specified')

    if args.path is not None and args.conf is not None:
        raise ValueError('Both path to config and version were specified')

    if args.path is not None:
        return args.path

    version   = args.conf
    conf_path = files('post_ap_data').joinpath(f'post_ap/{version}.yaml')
    conf_path = str(conf_path)

    if not os.path.isfile(conf_path):
        raise FileNotFoundError(f'Cannot find: {conf_path}')

    log.info(f'Using config: {conf_path}')

    return conf_path
# -------------------------------------------------
def _get_executable() -> Executable:
    runner_path = files('post_ap_grid').joinpath(Data.runner)
    runner_path = str(runner_path)

    obj          = Executable()
    obj.exe      = File(runner_path)
    obj.platform = Data.platform

    return obj
# -------------------------------------------------
def _get_output_files() -> list[Union[DiracFile,LocalFile]]:
    if Data.back in ['Interactive', 'Local']:
        return [LocalFile('*.root')]

    if Data.back in ['Dirac']:
        return [DiracFile('*.root')]

    raise ValueError(f'Invalid backend: {Data.back}')
# -------------------------------------------------
def _get_job() -> Job:
    job              = Job(name = Data.name)
    job.application  = _get_executable()
    job.inputfiles   = [ LocalFile(Data.conf), LocalFile(Data.pfn_path), DiracFile(Data.env_lfn) ]
    job.splitter     = _get_splitter()
    job.backend      = _get_backend()
    job.outputfiles  = _get_output_files()

    return job
# -------------------------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    if Data.dry_run:
        return

    job=_get_job()
    job.prepare()
    job.submit()
# -------------------------------------------------
if __name__ == 'GangaCore.GPI':
    main()
