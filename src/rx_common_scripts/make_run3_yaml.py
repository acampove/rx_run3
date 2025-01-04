'''
Script that will create list of samples for each project for Run3
based on file for Run1/2
'''
# pylint: disable=import-error, line-too-long

from typing              import Union
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import ap_utilities.decays.utilities as aput
from dmu.logging.log_store   import LogStore

log=LogStore.add_logger('rx_common:utilities')
# ----------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    l_data = [
            'DATA_24_MagDown_24c1',
            'DATA_24_MagDown_24c2',
            'DATA_24_MagDown_24c3',
            'DATA_24_MagDown_24c4',
            'DATA_24_MagUp_24c1',
            'DATA_24_MagUp_24c2',
            'DATA_24_MagUp_24c3',
            'DATA_24_MagUp_24c4',
            ]
# ----------------------------------
def _load_run12() -> dict:
    file_path = files('rx_config').joinpath('sample_run12.yaml')
    file_path = str(file_path)

    with open(file_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# ----------------------------------
def _run12_to_run3(sample : str) -> Union[str,None]:
    '''
    Takes sample name for Run12 and returns the naming for Run3
    '''
    try:
        nickname = aput.new_from_old_nick(nickname=sample)
    except ValueError as exc:
        log.warning(exc)
        nickname = None

    return nickname
# ----------------------------------
def main():
    '''
    Script starts here
    '''
    d_run12 = _load_run12()

    d_run3  = {}
    for proj, l_sample in d_run12.items():
        l_sample_run3 = [ _run12_to_run3(sample) for sample in l_sample                           ]
        d_run3[proj]  = [ sample                 for sample in l_sample_run3 if sample is not None]

    d_run3 = { proj : l_samp + Data.l_data for proj, l_samp in d_run3.items()}

    yaml_path = files('rx_config').joinpath('sample_run3.yaml')
    yaml_path = str(yaml_path)
    with open(yaml_path, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_run3, ofile)
# ----------------------------------
if __name__ == '__main__':
    main()
