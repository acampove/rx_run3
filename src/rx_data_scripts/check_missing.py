'''
Script used to check what samples are present in the main trees
but missing among the friend trees
'''
import os
import re
import glob
import argparse
from typing import Union

import yaml
from dmu.generic                    import utilities          as gut
from dmu.generic.version_management import get_last_version
from dmu.logging.log_store          import LogStore

log = LogStore.add_logger('rx_data:check_missing')
# ---------------------------------
class Data:
    '''
    Data class
    '''
    skip_sam : list[str]
    log_level: int
    data_dir : str = None
    data_rgx = r'(data_24_mag(?:down|up)_24c\d)_(.*)\.root'
    mc_rgx   = r'mc_mag(?:up|down)_(?:.*_)?\d{8}_(.*)_(Hlt2RD.*)_\w{10}\.root'
# ---------------------------------
def _set_data_dir() -> None:
    if Data.data_dir is not None:
        return

    if 'DATADIR' not in os.environ:
        raise FileNotFoundError('DATADIR environment variable not set')

    Data.data_dir = os.environ['DATADIR']
# ---------------------------------
def _initialize() -> None:
    _set_data_dir()
    LogStore.set_level('rx_data:check_missing', Data.log_level)
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script meant to check for missing friend trees')
    parser.add_argument('-d', '--data_dir' , type=str , help='Path to directory with main and friend samples, if not passed, will pick DATADIR from environment')
    parser.add_argument('-s', '--skip_sam' , nargs='+', help='Samples to skip', default=[])
    parser.add_argument('-l', '--log_level', type=int , help='Logging level', default=20, choices=[10, 20, 30])

    args = parser.parse_args()

    Data.data_dir = args.data_dir
    Data.skip_sam = args.skip_sam
    Data.log_level= args.log_level
# ---------------------------------
def _version_from_path(path : str) -> Union[str,None]:
    try:
        version=get_last_version(dir_path=path, version_only=True)
    except ValueError:
        log.debug(f'No version found for {path}')
        return None

    return version
# ---------------------------------
def _fname_from_sample(path : str, version : str) -> set[str]:
    root_wc = f'{path}/{version}/*.root'
    l_path  = glob.glob(root_wc)
    l_fname = [ os.path.basename(path) for path in l_path ]

    if len(l_fname) == 0:
        log.warning(f'No file found in: {root_wc}')

    return set(l_fname)
# ---------------------------------
def _info_from_fname(fname : str) -> tuple[str,str]:
    if fname.startswith('data_'):
        rgx = Data.data_rgx
    elif fname.startswith('mc_'):
        rgx = Data.mc_rgx
    else:
        raise ValueError(f'File cannot be identified as MC or data: {fname}')

    mtch = re.match(rgx, fname)
    if not mtch:
        raise ValueError(f'Cannot extract sample and trigger from: {fname}')

    [sample, trigger] = mtch.groups()

    return sample, trigger
# ---------------------------------
def _fname_to_dict(s_fname : set[str]) -> dict[str,dict[str,list[str]]]:
    d_data = {}

    for fname in s_fname:
        sample, trigger = _info_from_fname(fname)

        if 'data' in sample:
            sample = f'{sample}_{trigger}'

        if sample not in d_data:
            d_data[sample] = {}

        if trigger not in d_data[sample]:
            d_data[sample] = {trigger : []}

        d_data[sample][trigger].append(fname)

    return d_data
# ---------------------------------
def _find_paths() -> dict[str,set[str]]:
    l_sample = glob.glob(f'{Data.data_dir}/*')
    d_fname  = {}

    l_msg = []
    for sample in l_sample:
        version = _version_from_path(path=sample)
        if not version:
            continue

        name = os.path.basename(sample)

        s_fname = _fname_from_sample(path=sample, version=version)
        d_fname[name] = _fname_to_dict(s_fname)
        nfname  = len(s_fname)

        l_msg.append(f'{name:<20}{version:<10}{nfname:<10}')

    log.info(40 * '-')
    log.info(f'{"Tree":<20}{"Latest":<10}{"Files":<10}')
    log.info(40 * '-')
    for msg in l_msg:
        log.info(msg)
    log.info(40 * '-')

    return d_fname
# ---------------------------------
def _get_sample_files(sample : dict[str,list[str]]) -> set[str]:
    l_path = []
    for paths in sample.values():
        l_path += paths

    return set(l_path)
# ---------------------------------
def _sample_difference(sample_1 : dict[str,list[str]], sample_2 : dict[str,list[str]]) -> list[str]:
    s_path_1 = _get_sample_files(sample_1)
    s_path_2 = _get_sample_files(sample_2)
    s_diff   = s_path_1 - s_path_2
    l_diff   = list(s_diff)

    return sorted(l_diff)
# ---------------------------------
def _compare_against_main(main_sam : dict[str,dict], frnd_sam : dict[str,dict]) -> dict[str]:
    s_main_sample = set(main_sam.keys())
    s_frnd_sample = set(frnd_sam.keys())
    s_diff        = s_main_sample - s_frnd_sample
    l_diff        = list(s_diff)
    l_diff        = sorted(l_diff)

    d_diff = { sample : 'all' for sample in l_diff }
    s_both = s_main_sample & s_frnd_sample
    for sample in s_both:
        m_sample = main_sam[sample]
        f_sample = frnd_sam[sample]

        l_path = _sample_difference(sample_1=m_sample, sample_2=f_sample)
        npath  = len(l_path)
        if npath == 0:
            continue

        d_diff[sample] = l_path

    return d_diff
# ---------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    d_sample = _find_paths()

    main_sam = d_sample['main']
    d_mis    = {}
    for friend, data in d_sample.items():
        if friend in ['main', 'samples']: # samples stores yaml files
            continue

        if friend in Data.skip_sam:
            continue

        d_mis[friend] = _compare_against_main(main_sam=main_sam, frnd_sam=data)

    with open('missing.yaml', 'w', encoding='utf-8') as ofile:
        yaml.dump(d_mis, ofile, Dumper=gut.BlockStyleDumper)
# ---------------------------------
if __name__ == '__main__':
    main()
