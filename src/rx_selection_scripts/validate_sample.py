'''
Script used to make validation plots
'''
# pylint: disable = import-error

import argparse
from dataclasses            import dataclass
from importlib.resources    import files

import yaml
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D
from dmu.plotting.plotter_2d import Plotter2D
from ROOT                    import RDataFrame
from rx_selection.ds_getter  import ds_getter

log = LogStore.add_logger('rx_selection:cache_data')
# --------------------------
@dataclass
class Data:
    '''
    Class used to store shared attributes
    '''
    cfg_val : dict
    cfg_sel : dict
    d_cut   : dict[str,str]

    version : str
    nparts  : int
# --------------------------
def _override_validation_config(cfg : dict) -> dict:
    hlt2   = cfg['sample']['hlt2']
    sample = cfg['sample']['sample']
    for plt_key in ['plotting_1d', 'plotting_2d']:
        val_dir = cfg[plt_key]['saving']['plt_dir']
        val_dir = f'{val_dir}/{sample}/{hlt2}'
        cfg[plt_key]['saving']['plt_dir'] = val_dir

    return cfg
# --------------------------
def _initialize():
    cfg_val      = _load_config(dir_name = 'validation', file_name = f'{Data.version}.yaml')
    Data.cfg_val = _override_validation_config(cfg_val)

    cut_ver      = Data.cfg_val['sample']['cutver']
    Data.cfg_sel = _load_config(dir_name = 'selection' , file_name = f'{cut_ver}.yaml')

    Data.d_cut   = _get_selection()
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
def _get_selection() -> dict[str,str]:
    project  = Data.cfg_val['sample']['project']
    trigger  = Data.cfg_val['sample']['hlt2']
    analysis = 'MM' if 'MuMu' in trigger else 'EE'

    d_cut    = Data.cfg_sel[project][analysis]

    return d_cut
# --------------------------
def _get_config() -> dict:
    d_cfg = {
            'npart'    : Data.nparts,
            'ipart'    : 0,
            'q2bin'    : 'central', # Just to make sure ds_getter does not complain, this cut will be removed later
            'redefine' : {cut : '(1)' for cut in Data.d_cut},
            }

    d_cfg.update(Data.cfg_val['sample'])

    return d_cfg
# --------------------------
def _get_samples(rdf : RDataFrame) -> dict[str,RDataFrame]:
    d_rdf = {'None' : rdf}
    for cut_name, cut_expr in Data.d_cut.items():
        if cut_name in ['q2', 'mass']:
            continue

        rdf = rdf.Filter(cut_expr, cut_name)
        d_rdf[cut_name] = rdf

    return d_rdf
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

    cfg_plt = Data.cfg_val['plotting_2d']
    ptr=Plotter2D(rdf=rdf, cfg=cfg_plt)
    ptr.run()

    d_rdf   = _get_samples(rdf)
    cfg_plt = Data.cfg_val['plotting_1d']
    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg_plt)
    ptr.run()
# --------------------------
if __name__ == '__main__':
    main()
