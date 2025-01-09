'''
Script used to make validation plots
'''
# pylint: disable = import-error

import argparse
from dataclasses            import dataclass
from importlib.resources    import files
from functools              import cache

import yaml
from dmu.logging.log_store  import LogStore
from ROOT                   import RDataFrame
from rx_selection.ds_getter import ds_getter

log = LogStore.add_logger('rx_selection:cache_data')
# --------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    version : str
    nparts  : int
# --------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to validate ntuples')
    parser.add_argument('-v', '--version' , type=str, help='Version of validation configuration'   , required=True)
    parser.add_argument('-n', '--nparts'  , type=int, help='Number of parts to split validation on, default 1', default=1)
    args = parser.parse_args()

    Data.version = args.version
    Data.nparts  = args.nparts
# --------------------------
@cache
def _load_config(dir_name : str, file_name : str) -> dict:
    cfg_path = files('rx_selection_data').joinpath(f'{dir_name}/{file_name}')
    cfg_path = str(cfg_path)

    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# --------------------------
def _get_config() -> dict:
    d_cfg = {
            'npart'    : Data.nparts,
            'ipart'    : 0,
            'q2bin'    : 'central', # Just to make sure ds_getter does not complain, this cut will be removed later
            'redefine' : {},
            }

    d_ext = _load_config()

    d_sam = d_ext['sample']
    d_cfg.update(d_sam)

    pprint.pprint(d_cfg)

    return d_cfg
# --------------------------
def _validate(rdf : RDataFrame) -> None:
    pass
# --------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    cfg = _get_config()
    dsg = ds_getter(cfg=cfg)
    rdf = dsg.get_rdf()

    _validate(rdf)
# --------------------------
if __name__ == '__main__':
    main()
