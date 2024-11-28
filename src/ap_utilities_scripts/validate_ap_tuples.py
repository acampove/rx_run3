'''
Script used to validate ntuples produced by AP pipelines
'''
import os
import glob
import shutil
import argparse
from typing              import Union
from typing              import ClassVar
from dataclasses         import dataclass
from concurrent.futures  import ThreadPoolExecutor, as_completed

import tqdm
import yaml
import pandas as pnd

from ROOT                            import TFile, TDirectoryFile, TTree
from ap_utilities.logging.log_store  import LogStore
from ap_utilities.logfiles.log_info  import LogInfo

log = LogStore.add_logger('ap_utilities_scripts:validate_ap_tuples')
# -------------------------------
@dataclass
class Data:
    '''
    Class holding shared attributes
    '''
    pipeline_id : int
    config_path : str
    nthread     : int
    cfg         : dict

    d_tree_miss      : ClassVar[dict[str, list[str]]]     = {}
    d_tree_found     : ClassVar[dict[str, list[str]]]     = {}
    d_tree_entries   : ClassVar[dict[str, dict[str,int]]] = {}
    d_mcdt           : ClassVar[dict[str, dict[str,int]]] = {}
    d_sample_entries : ClassVar[dict[str,int]]            = {}
    l_missing_job    : ClassVar[list[str]]                = []
    d_log_stat       : ClassVar[dict[str,int]]            = {}
    d_root_stat      : ClassVar[dict[str,int]]            = {}
# -------------------------------
def _check_path(path : str) -> None:
    found = os.path.isdir(path) or os.path.isfile(path)
    if not found:
        raise FileNotFoundError(f'Cannot find: {path}')
# -------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Makes a list of PFNs for a specific set of eventIDs in case we need to reprocess them')
    parser.add_argument('-p','--pipeline', type=int, help='Pipeline ID', required=True)
    parser.add_argument('-f','--cfg_path', type=str, help='Path to config file with the description of how to validate', required=True)
    parser.add_argument('-l','--log_lvl' , type=int, help='Logging level', default=20, choices=[10,20,30])
    parser.add_argument('-t','--nthread' , type=int, help='Number of threads', default=1)
    args = parser.parse_args()

    Data.pipeline_id = args.pipeline
    Data.config_path = args.cfg_path
    Data.nthread     = args.nthread

    LogStore.set_level('ap_utilities_scripts:validate_ap_tuples', args.log_lvl)
# -------------------------------
def _load_config() -> None:
    if not os.path.isfile(Data.config_path):
        raise FileNotFoundError(f'Could not find: {Data.config_path}')

    with open(Data.config_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)
# -------------------------------
def _get_out_paths() -> list[str]:
    '''
    Returns list of paths containing the ROOT files and the zip file with the logs
    '''
    pipeline_dir = Data.cfg['paths']['pipeline_dir']
    analysis_dir = Data.cfg['paths']['analysis_dir']

    job_path = f'{pipeline_dir}/{Data.pipeline_id}/{analysis_dir}'
    _check_path(job_path)

    sample_wc = f'{job_path}/*/*'
    l_sample  = glob.glob(sample_wc)

    nsample   = len(l_sample)
    njobs     = len(Data.cfg['samples'])

    if nsample != njobs:
        log.warning(f'Number of samples and jobs in {sample_wc} differ: {nsample} -> {njobs}')

    return l_sample
# -------------------------------
def _get_file_path(job_path : str, ending : str) -> Union[str,None]:
    path_wc = f'{job_path}/*{ending}'
    try:
        [file_path] = glob.glob(path_wc)
    except ValueError:
        log.debug(f'Cannot find one and only one file in: {path_wc}')
        return None

    return file_path
# -------------------------------
def _sample_from_path(path : str) -> str:
    '''
    Picks up path to file, returns sample associated to path
    '''
    l_samp = Data.cfg['samples']
    try:
        [samp] = [ sample for sample in l_samp if sample in path ]
    except ValueError as exc:
        raise ValueError(f'Not found one and only one sample corresponding to: {path}') from exc

    return samp
# -------------------------------
def _copy_path(source : str) -> str:
    target = f'/tmp/{source}'
    target = target.replace('#', '_')

    if os.path.isfile(target):
        return target

    target_dir = os.path.dirname(target)
    os.makedirs(target_dir, exist_ok=True)

    log.debug(f'{source} --> {target}')
    shutil.copy(source, target)

    return target
# -------------------------------
def _validate_root_file(root_path : str) -> None:
    _validate_trees(root_path)
# -------------------------------
def _add_to_dictionary(d_data : dict, identifier : str, key : str, value : int) -> None:
    if identifier not in d_data:
        d_data[identifier] = {}

    if key in d_data[identifier]:
        log.warning(f'Overriding key {key} at {identifier}')

    d_data[identifier][key] = value
# -------------------------------
def _is_valid_reco_dir(sample : str, file_dir : TDirectoryFile) -> bool:
    if hasattr(file_dir, 'MCDecayTree'):
        nentries = file_dir.MCDecayTree.GetEntries()
        _add_to_dictionary(Data.d_tree_entries, sample, key=file_dir.GetName(), value=nentries)
        return False

    if not hasattr(file_dir, 'DecayTree') or not isinstance(file_dir.DecayTree, TTree):
        _add_to_dictionary(Data.d_tree_entries, sample, key=file_dir.GetName(), value=0)
        return False

    nentries = file_dir.DecayTree.GetEntries()
    if nentries == 0:
        _add_to_dictionary(Data.d_tree_entries, sample, key=file_dir.GetName(), value=0)
        return False

    _add_to_dictionary(Data.d_tree_entries, sample, key=file_dir.GetName(), value=nentries)

    return True
# -------------------------------
def _check_mcdt_entries(sample : str, l_dir : list[TDirectoryFile]) -> dict[str,int]:
    '''
    Given a sample and a list of directories with a tree each
    If the MCDecayTree is not found return None, if it is found and the entries agree with what is in d_sample_entries
    return True, if they do not agree, return false
    '''
    l_mcdir   = [ directory for directory in l_dir if directory.GetName() == sample ]

    nexpected= Data.d_sample_entries[sample]
    if len(l_mcdir) == 0:
        return {'Expected' : nexpected, 'Found' : -1}

    tree     = l_mcdir[0].MCDecayTree
    nfound   = tree.GetEntries()

    return {'Expected' : nexpected, 'Found' : nfound}
# -------------------------------
def _validate_trees(root_path : str) -> None:
    sample    = _sample_from_path(root_path)
    l_expected= Data.cfg['samples'][sample]
    s_expected= set(l_expected)

    root_path = _copy_path(root_path)
    rfile     = TFile(root_path)
    l_key     = rfile.GetListOfKeys()
    l_dir     = [ key.ReadObj() for key in l_key if key.ReadObj().InheritsFrom('TDirectoryFile') ]
    s_found   = { fdir.GetName() for fdir in l_dir if _is_valid_reco_dir(sample, fdir)}

    Data.d_mcdt[sample] = _check_mcdt_entries(sample, l_dir)

    if s_expected == {'any'} and len(s_found) > 0:
        _save_trees(sample, s_found, Data.d_tree_found)
        return

    s_missing = s_expected - s_found

    if len(s_missing) > 0:
        log.warning(f'File: {root_path}')
        log.warning(f'Missing : {s_missing}')
        _save_trees(sample, s_missing, Data.d_tree_miss )

    _save_trees(sample, s_found, Data.d_tree_found)
# -------------------------------
def _save_trees(sample : str, s_tree_name : set[str], d_data : dict[str, list[str]]):
    l_tree_name = list(s_tree_name)
    d_data.update({sample : l_tree_name})
# -------------------------------
def _update_sample_stats(log_path : str) -> None:
    sample   = _sample_from_path(log_path)
    obj      = LogInfo(zip_path = log_path)

    Data.d_sample_entries[sample] = obj.get_mcdt_entries(sample, fall_back=-1)
# -------------------------------
def _check_job(sample : str, log_path : Union[str,None], root_path : Union[str,None]):
    if log_path is None:
        Data.d_log_stat[sample] = -1
    else:
        Data.d_log_stat[sample] = +1

    if root_path is None:
        Data.d_root_stat[sample] = -1
    else:
        Data.d_root_stat[sample] = +1
# -------------------------------
def _validate_job(job_path : str) -> None:
    '''
    Picks path to directory with ROOT and zip file
    Runs validation
    '''

    root_path = _get_file_path(job_path, ending='_2.tuple.root')
    log_path  = _get_file_path(job_path, ending=         '.zip')

    sample    = _sample_from_path(job_path)
    _check_job(sample, log_path, root_path)
    if log_path is None or root_path is None:
        Data.l_missing_job.append(job_path)
        return

    _update_sample_stats(log_path)
    _validate_root_file(root_path)
# -------------------------------
def _validate() -> None:
    _load_config()
    l_out_path = _get_out_paths()

    npath = len(l_out_path)
    log.info(f'Checking {npath} jobs')

    if Data.nthread > 1:
        _validate_with_multithreading(l_out_path)
        return

    log.info('Using single thread')
    for out_path in tqdm.tqdm(l_out_path, ascii=' -'):
        _validate_job(out_path)
# -------------------------------
def _validate_with_multithreading(l_out_path : list[str]) -> None:
    npath = len(l_out_path)
    with ThreadPoolExecutor(max_workers=Data.nthread) as executor:
        l_feat = [ executor.submit(_validate_job, out_path) for out_path in l_out_path ]

        for _ in tqdm.tqdm(as_completed(l_feat), total=npath, ascii=' -'):
            ...
# -------------------------------
def _get_mcdt_dataframe() -> pnd.DataFrame:
    l_sample   = []
    l_found    = []
    l_expected = []
    l_log_stat = []
    l_root_stat= []

    nsample = len(Data.d_mcdt)
    log.info(f'Found {nsample} samples with MCDT')

    for sample, d_stat in Data.d_mcdt.items():
        expected = d_stat['Expected']
        found    = d_stat['Found'   ]
        miss_root= Data.d_root_stat[sample]
        miss_log = Data.d_log_stat[sample]

        l_sample.append(sample)
        l_found.append(found)
        l_expected.append(expected)
        l_root_stat.append(miss_root)
        l_log_stat.append(miss_log)

    df = pnd.DataFrame({'Sample' : l_sample, 'Found' : l_found, 'Expected' : l_expected, 'Has log' : l_log_stat, 'Has tree' : l_root_stat})

    df['Difference [%]'] = 100 * (df.Expected - df.Found).abs() / df.Expected

    df = df.sort_values(by='Difference [%]', ascending=False)

    return df
# -------------------------------
def _save_report() -> None:
    d_rep = {
            'missing_trees'    : Data.d_tree_miss,
            'found_trees'      : Data.d_tree_found,
            'tree_entries'     : Data.d_tree_entries,
            'missing_jobs'     : Data.l_missing_job,
            }

    with open(f'report_{Data.pipeline_id}.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_rep, ofile, sort_keys=False, indent=2, default_flow_style=False)

    df = _get_mcdt_dataframe()
    with open(f'mcdt_{Data.pipeline_id}.md', 'w', encoding='utf-8') as ofile:
        df.to_markdown(ofile, index=False)
# -------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _validate()
    _save_report()
# -------------------------------
if __name__ == '__main__':
    main()
