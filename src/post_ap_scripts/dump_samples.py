'''
Script used to provide blocks of MC samples in a YAML file
Needed to write YAML config for post_ap
'''
# pylint: disable = too-many-ancestors

import re
import argparse

import yaml
from apd import AnalysisData
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('dmu:post_ap_scripts:dump_samples')
# ----------------------------------------------
class IndentedDumper(yaml.Dumper):
    '''
    Class needed to create indentation when saving lists as values of dictionaries in YAML files
    '''
    def increase_indent(self, flow=False, indentless=False):
        return super().increase_indent(flow, False)
# ----------------------------------------------
class Data:
    '''
    Data class used to store shared data
    '''
    regex = r'mc_\d{2}_(w\d{2}_\d{2})_.*'
    vers  : str
    group : str
    prod  : str
# ----------------------------------------------
def _version_from_name(name : str) -> str:
    mtch = re.match(Data.regex, name)
    if not mtch:
        raise ValueError(f'Cannot find version in: {name}')

    return mtch.group(1)
# ----------------------------------------------
def _get_samples(samples) -> dict[str,list[str]]:
    d_data   = {}
    for sample in samples:
        if sample['version'] != Data.vers:
            continue

        name = sample['name']
        if not name.startswith('mc_'):
            continue

        vers = _version_from_name(name)
        if vers not in d_data:
            d_data[vers] = []

        d_data[vers].append(name)

    if len(d_data) == 0:
        raise ValueError('No samples found')

    for l_sam in d_data.values():
        l_sam.sort()

    return d_data
# ----------------------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to create a list of MC samples in YAML, split by sim production for a given (latest) version of the AP.')
    parser.add_argument('-v', '--vers' , type=str, help='Version of AP, e.g. v1r2266', required=True)
    parser.add_argument('-p', '--prod' , type=str, help='Production, e.g. rd_ap_2024', required=True)
    parser.add_argument('-g', '--group', type=str, help='Group, e.g. rd'             , required=True)
    args = parser.parse_args()

    Data.vers  = args.vers
    Data.group = args.group
    Data.prod  = args.prod
# ----------------------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()

    datasets = AnalysisData(Data.group, Data.prod)
    samples  = datasets.all_samples()

    d_data = _get_samples(samples)
    with open(f'{Data.group}_{Data.prod}_{Data.vers}.yaml', 'w', encoding='utf-8') as ofile:
        yaml.dump(d_data, ofile, Dumper=IndentedDumper)
# ----------------------------------------------
if __name__ == '__main__':
    main()
