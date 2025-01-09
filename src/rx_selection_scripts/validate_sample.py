'''
Script used to make validation plots
'''
# pylint: disable = import-error

import argparse
from dataclasses            import dataclass
from importlib.resources    import files

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
    cfg_val : dict
    cfg_sel : dict

    version : str
    nparts  : int
# --------------------------
def _initialize():
    Data.cfg_val = _load_config(dir_name = 'validation', file_name = f'{Data.version}.yaml')

    cut_ver      = Data.cfg_val['sample']['cutver']
    Data.cfg_sel = _load_config(dir_name = 'selection' , file_name = f'{cut_ver}.yaml')
# --------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to validate ntuples')
    parser.add_argument('-v', '--version' , type=str, help='Version of validation configuration'   , required=True)
    parser.add_argument('-n', '--nparts'  , type=int, help='Number of parts to split validation on, default 1', default=1)
    args = parser.parse_args()

    Data.version = args.version
    Data.nparts  = args.nparts
# --------------------------
def _load_config(dir_name : str, file_name : str) -> dict:
    cfg_path = files('rx_selection_data').joinpath(f'{dir_name}/{file_name}')
    cfg_path = str(cfg_path)

    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# --------------------------
def _get_redefinitions() -> dict[str,str]:
    project  = Data.cfg_val['sample']['project']
    trigger  = Data.cfg_val['sample']['hlt2']
    analysis = 'MM' if 'MuMu' in trigger else 'EE'

    d_cut    = Data.cfg_sel[project][analysis]
    d_rem    = {cut_name : '(1)' for cut_name in d_cut}

    return d_rem
# --------------------------
def _get_config() -> dict:
    d_cfg = {
            'npart'    : Data.nparts,
            'ipart'    : 0,
            'q2bin'    : 'central', # Just to make sure ds_getter does not complain, this cut will be removed later
            'redefine' : _get_redefinitions(),
            }

    d_cfg.update(Data.cfg_val['sample'])

    return d_cfg
# --------------------------
def _add_columns(rdf : RDataFrame) -> RDataFrame:
    log.info('Defining columns')

    d_def = Data.cfg_val['definitions']
    for var_name, var_expr in d_def.items():
        rdf = rdf.Define(var_name, var_expr)

    return rdf
# --------------------------
def _validate(rdf : RDataFrame, var : str) -> None:
    pass
# --------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _initialize()

    cfg = _get_config()
    dsg = ds_getter(cfg=cfg)
    rdf = dsg.get_rdf()
    rdf = _add_columns(rdf)

    for var in Data.cfg_val['variables']:
        _validate(rdf, var)
# --------------------------
if __name__ == '__main__':
    main()
