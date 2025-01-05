'''
Module that will transform lists of LFNs from AP jobs
into a yaml file ready to be plugged into the RX c++ framework
'''
# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name
import os
import re
from dataclasses            import dataclass
from importlib.resources    import files

import yaml
import ap_utilities.decays.utilities as aput
from dmu.logging.log_store  import LogStore
from post_ap.pfn_reader     import PFNReader

log = LogStore.add_logger('rx_common:pap_lfn_to_yaml')

Project=dict[str,str]
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    production= 'rd_ap_2024'
    out_dir   = 'samples'
    l_project = ['RK', 'RKst']
    regex_sam = r'mc_24_w31_34_hlt1bug_magup_sim10d(-splitsim02)?_\d{8}_(.*)'

    d_sample_pfn : dict[str,list[str]]
# ---------------------------------
def _initialize() -> None:
    os.makedirs(Data.out_dir, exist_ok=True)
    Data.d_sample_pfn = _get_pfns()
# ---------------------------------
def _get_pfns() -> dict[str,list[str]]:
    '''
    Returns dictionary with sample name mapped to list of PFNs
    '''
    log.info('Loading PFNs')
    cfg    = _load_config(project='post_ap_data', name='post_ap/v3.yaml')
    d_samp = cfg['productions'][Data.production]['samples']
    l_samp = list(d_samp)

    reader = PFNReader(cfg=cfg)
    d_pfn  = {}
    for samp in l_samp:
        d_tmp = reader.get_pfns(production=Data.production, nickname=samp)
        d_pfn.update(d_tmp)

    return d_pfn
# ---------------------------------
def _get_metadata(project : str) -> dict[str,str]:
    return {
            'MCDTName'     : 'MCDecayTree',
            'DTName'       : 'DecayTree',
            'LumiTreeName' : 'LumiTree',
            }
# ---------------------------------
def _load_config(project : str, name : str) -> dict:
    '''
    Loads config file in a given directory, using its name as argument
    '''
    cfg_path = files(project).joinpath(name)
    cfg_path = str(cfg_path)

    with open(cfg_path, encoding='utf-8') as ifile:
        d_conf = yaml.safe_load(ifile)

    return d_conf
# ---------------------------------
def _samples_from_project(project : str) -> list[str]:
    d_prj_sample = _load_config('rx_config', 'sample_run3.yaml')

    if project not in d_prj_sample:
        raise ValueError(f'Project {project} not found')

    return d_prj_sample[project]
# ---------------------------------
def _path_from_sample(sample : str) -> str:
    '''
    Takes name of sample, retrieves list of PFNs,
    writes them to a text file,
    returns path to text file with the lists of PFNs corresponding to that sample
    '''
    if sample not in Data.d_sample_pfn:
        raise ValueError(f'Cannot find sample {sample}')

    l_pfn = Data.d_sample_pfn[sample]
    pfn_list = '\n'.join(l_pfn)
    pfn_path = f'{Data.out_dir}/{sample}.txt'
    with open(pfn_path, 'w', encoding='utf-8') as ofile:
        ofile.write(pfn_list)

    return pfn_path
# ---------------------------------
def _get_project_data(project : str) -> Project:
    l_sample = _samples_from_project(project)

    prj = { sample : _path_from_sample(sample) for sample in l_sample}

    d_meta = _get_metadata(project)
    prj.update(d_meta)

    return d_meta
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _initialize()

    d_data = { project : _get_project_data(project) for project in Data.l_project }

    with open('samples.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_data, ofile)
# ---------------------------------
if __name__ == '__main__':
    main()
