'''
Module containing utility functions needed by unit tests
'''

from importlib.resources import files

import yaml

# -------------------------------
def get_config(name : str | None = None):
    '''
    Takes path to the YAML config file, after `dmu_data`
    Returns dictionary with config
    '''
    if name is None:
        raise ValueError('Name not pased')

    cfg_path = files('dmu_data').joinpath(name)
    cfg_path = str(cfg_path)
    with open(cfg_path, encoding='utf-8') as ifile:
        d_cfg = yaml.safe_load(ifile)

    return d_cfg
# -------------------------------
