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
from typing                 import Union

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
    trg_kind  = 'mva_turbo'
    production= 'rd_ap_2024'
    out_dir   = 'samples'
    l_project = ['RK', 'RKst']
    regex_sam = r'mc_24_w31_34_hlt1bug_magup_sim10d(-splitsim02)?_\d{8}_(.*)'

    l_sam_ee = ['Bd_Kstee_eq_btosllball05_DPC',
                'Bu_Kee_eq_btosllball05_DPC',
                'Bd_Dmnpipl_eq_DPC',
                'Bd_Denu_Kstenu_eq_VIA_HVM_EGDWC',
                'Bd_Ksteta_eplemng_eq_Dalitz_DPC',
                'Bd_Kstgamma_eq_HighPtGamma_DPC',
                'Bd_Kstpi0_eplemng_eq_Dalitz_DPC',
                'Bs_phiee_eq_Ball_DPC',
                'Bu_K1ee_eq_DPC',
                'Bu_K2stee_Kpipi_eq_mK1430_DPC',
                'Bd_Dstplenu_eq_PHSP_TC']

    l_sam_mm = ['Bd_Kstmumu_eq_btosllball05_DPC',
                'Bu_Kmumu_eq_btosllball05_DPC',
                'Bu_KpipiJpsi_mumu_eq_DPC_LSFLAT',
                'Bs_phimumu_eq_Ball_DPC']

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

    d_pfn_renamed = _rename_samples(d_pfn)

    return d_pfn_renamed
# ---------------------------------
def _name_from_sample(sample : str) -> str:
    sample = sample.replace('_full_' , '_')
    sample = sample.replace('_turbo_', '_')
    sample = sample.replace(',_tuple',  '')
    sample = sample.replace('_tuple' ,  '')

    if sample.startswith('data_'):
        return sample

    if 'tuple' in sample:
        raise ValueError(f'Invalid sample name: {sample}')

    mtch   = re.match(Data.regex_sam, sample)
    if not mtch:
        raise ValueError(f'Cannot extract sample name from {sample}')

    return mtch.group(2)
# ---------------------------------
def _rename_samples(d_pfn : dict[str,list[str]]) -> dict[str,list[str]]:
    d_pfn_renamed = {}
    for sample, l_pfn in d_pfn.items():
        sample = _name_from_sample(sample)
        sample = aput.name_from_lower_case(sample)

        if sample not in d_pfn_renamed:
            d_pfn_renamed[sample] = []

        d_pfn_renamed[sample] += l_pfn

    return d_pfn_renamed
# ---------------------------------
def _get_trigger_name(project : str, channel : str) -> str:
    if [project, channel, Data.trg_kind] == ['RK', 'ee', 'mva_turbo']:
        return 'Hlt2RD_BuToKpEE_MVA'

    if [project, channel, Data.trg_kind] == ['RK', 'mm', 'mva_turbo']:
        return 'Hlt2RD_BuToKpMuMu_MVA'

    if [project, channel, Data.trg_kind] == ['RKst', 'ee', 'mva_turbo']:
        return 'Hlt2RD_B0ToKpPimEE_MVA'

    if [project, channel, Data.trg_kind] == ['RKst', 'mm', 'mva_turbo']:
        return 'Hlt2RD_B0ToKpPimMuMu_MVA'

    raise ValueError(f'Invalid project/channel/trigger combination: {project}/{channel}/{Data.trg_kind}')
# ---------------------------------
def _get_metadata(project : str, channel : str) -> dict[str,str]:
    if project not in Data.l_project:
        raise ValueError(f'Invalid project: {project}')

    trigger_name = _get_trigger_name(project, channel)

    return {
            'MCDTName'     : 'MCDecayTree',
            'DTName'       : f'{trigger_name}/DecayTree',
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
def _path_from_sample(sample : str) -> Union[str,None]:
    '''
    Takes name of sample, retrieves list of PFNs,
    writes them to a text file,
    returns path to text file with the lists of PFNs corresponding to that sample
    If sample was not found in the AP list, will show warning and return None
    '''
    if sample not in Data.d_sample_pfn:
        log.warning(f'Cannot find sample {sample}')
        return None

    l_pfn = Data.d_sample_pfn[sample]
    pfn_list = '\n'.join(l_pfn)
    pfn_path = f'{Data.out_dir}/{sample}.txt'
    with open(pfn_path, 'w', encoding='utf-8') as ofile:
        ofile.write(pfn_list)

    return pfn_path
# ---------------------------------
def _is_channel(sample : str, channel : str) -> bool:
    '''
    Takes sample name and channel, ee or mm.
    Returns True or false, depending on wether sample belongs to channel
    '''
    if sample.startswith('DATA_'):
        return True

    if '_ee_' in sample and channel == 'ee':
        return True

    if '_mm_' in sample and channel == 'ee':
        return False

    if '_ee_' in sample and channel == 'mm':
        return False

    if '_mm_' in sample and channel == 'mm':
        return True

    if channel == 'ee' and sample in Data.l_sam_ee:
        return True

    if channel == 'mm' and sample in Data.l_sam_ee:
        return False

    if channel == 'mm' and sample in Data.l_sam_mm:
        return True

    if channel == 'ee' and sample in Data.l_sam_mm:
        return False

    raise ValueError(f'Cannot determine channel for: {sample}')
# ---------------------------------
def _get_project_data(project : str) -> tuple[Project,Project]:
    log.info(f'Getting data for: {project}')
    l_sample = _samples_from_project(project)

    prj = { sample : _path_from_sample(sample) for sample in l_sample}
    prj = { sample : path for sample, path in prj.items() if path is not None}

    nsample = len(prj)
    log.info(f'Found {nsample} samples')

    prj_mm = {sample : list_path for sample, list_path in prj.items() if _is_channel(sample, channel='mm')}
    prj_ee = {sample : list_path for sample, list_path in prj.items() if _is_channel(sample, channel='ee')}

    prj_mm.update(_get_metadata(project, channel='mm'))
    prj_ee.update(_get_metadata(project, channel='ee'))

    return prj_mm, prj_ee
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _initialize()

    d_data = {}
    for project in Data.l_project:
        prj_mm, prj_ee = _get_project_data(project)
        d_data[f'{project}-MM'] = prj_mm
        d_data[f'{project}-EE'] = prj_ee

    with open('samples.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_data, ofile)
# ---------------------------------
if __name__ == '__main__':
    main()
