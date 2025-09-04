'''
Script in charge of:

- Loading YAML file with event types
- Checking which classified event types do not belong to the all section
- Checking which event types are in the all section but not in the classified one
- Save summary to YAML
'''

import argparse

from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore
from omegaconf             import DictConfig
from ap_utilities.decays   import utilities as aput

log=LogStore.add_logger('ap_utilities:analyze_samples')
# ----------------------
def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Script used to update config with event types')
    parser.add_argument('-c', '--config' , type=str, help='Name of config, e.g. by_priority', required=True)
    args = parser.parse_args()

    return args
# ----------------------
def _analyze_conf(cfg : DictConfig) -> None:
    '''
    This method creates YAML file with information mentioned in scripts main docstring

    Parameters
    -------------
    cfg: Object storing event types, etc
    '''
    d_new = {}
    for section, d_evt_type in cfg.items():
        if section in ['all', 'missing']:
            continue

        tmp = { key : val for key, val in d_evt_type.items() if key not in cfg.all }
        d_new.update(tmp)

    d_missing = {}
    for evt_type in cfg.all:
        if _found_in_any_section(evt_type=evt_type, cfg=cfg):
            continue

        d_missing[evt_type] = aput.read_decay_name(event_type=evt_type)

    d_missing = dict(sorted(d_missing.items(), key=lambda item: item[1]))
    d_summary = { 'new' : d_new, 'missing' : d_missing }
    gut.dump_json(data=d_summary, path='./summary.yaml')
# ----------------------
def _found_in_any_section(evt_type : int, cfg : DictConfig) -> bool:
    '''
    Parameters
    -------------
    evt_type: Event type
    cfg: Config loaded from YAML file

    Returns
    -------------
    True if evt_type is found in any of the sections, excluding `all`
    '''
    for section in cfg:
        if section == 'all':
            continue

        data = cfg[section]

        if evt_type in data:
            return True

    return False
# ----------------------
def _analyze_missing(cfg : DictConfig) -> None:
    '''
    Parameters
    -------------
    cfg: Config with samples from YAML file
    '''
    data = {}
    for block, event_types in cfg.missing.items():
        data[block] = { etype : aput.read_decay_name(event_type=etype) for etype in event_types }

    gut.dump_json(data=data, path='./missing.yaml')
# ----------------------
def main():
    '''
    Entry point
    '''
    args = _parse_args()
    cfg  = gut.load_conf(package='ap_utilities_data', fpath=f'analyses/{args.config}.yaml')

    _analyze_conf(cfg=cfg)
    _analyze_missing(cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
