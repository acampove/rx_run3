'''
Script meant to be used to create PID maps
'''
# pylint: disable=line-too-long
import os
import argparse
from typing import cast

from omegaconf                import DictConfig, OmegaConf 
from pidcalib2.make_eff_hists import make_eff_hists

import dmu.generic.utilities as gut
from dmu.generic             import hashing
from dmu.logging.log_store   import LogStore

log=LogStore.add_logger('rx_pid:create_pid_maps')
# --------------------------------
class Data:
    '''
    Data class
    '''
    l_particle : list[str] = ['e', 'Pi', 'K', 'Mu', 'P']

    kind    : str
    brem    : str
    particle: str
    sample  : str
    out_dir : str
    region  : str
    dry_run : bool

    max_files: int
    verbose  : bool
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to calculate PID efficiencies using PIDCalib2')
    parser.add_argument('-k', '--kind'    , type=str, help='Kind of map', choices=['signal', 'control'], required=True)
    parser.add_argument('-b', '--brem', type=str, help='Version of binning file'                   , required=True)
    parser.add_argument('-p', '--particle', type=str, help='Particle name', choices=Data.l_particle    , required=True)
    parser.add_argument('-s', '--sample'  , type=str, help='Sample/block, e.g. b1, b2...'              , required=True)
    parser.add_argument('-o', '--out_dir' , type=str, help='Directory where pkl files will go'         , required=True)
    parser.add_argument('-r', '--region'  , type=str, help='Used to define selection', choices=['signal', 'control'], required=True)
    parser.add_argument('-d', '--dry-run' ,           help='Enable dry-run mode (default: False)'      , action='store_true')
    # These are by default None and will be used as in PIDCalib2's make_eff_hists
    parser.add_argument('-M', '--maxfiles', type=int, help='Limit number of files to this value')
    parser.add_argument('-v', '--verbose' , help='Will print debug messages', action='store_true')

    args          = parser.parse_args()
    Data.kind     = args.kind
    Data.brem     = args.brem
    Data.region   = args.region
    Data.out_dir  = args.out_dir
    Data.particle = args.particle
    Data.sample   = args.sample
    Data.dry_run  = args.dry_run
    Data.verbose  = args.verbose
    Data.max_files= args.maxfiles
# --------------------------------
def _get_bin_vars() -> str:
    if not hasattr(Data, 'conf'):
        raise AttributeError('Data class does not have a config dictionary')

    l_var   = Data.conf['bin_vars']
    var_str = '_'.join(l_var)

    return var_str
# --------------------------------
def _path_from_kind(kind : str) -> str:
    if   kind == 'config':
        version = Data.cfg_vers
        path    = files('rx_pid_data').joinpath(f'{kind}/{Data.kind}/{version}.yaml')
        path    = str(path)
    elif kind == 'binning':
        version = Data.bin_vers
        bin_vars= _get_bin_vars()
        path    = files('rx_pid_data').joinpath(f'{kind}/{bin_vars}/{version}.yaml')
        path    = str(path)
        path    = _replace_binning_path(path)
    else:
        raise ValueError(f'Invalid kind: {kind}')

    return path
# --------------------------------
def _replace_binning_path(yaml_path : str) -> str:
    with open(yaml_path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    data     = data[Data.sample]
    hash_val = hashing.hash_object(data)

    os.makedirs('.binning', exist_ok=True)

    json_path = f'.binning/{hash_val}.json'
    log.debug(f'Using binning path: {json_path}')
    gut.dump_json(data, json_path)

    return json_path
# --------------------------------
def _load_data(kind : str) -> dict:
    path = _path_from_kind(kind)

    with open(path, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    return data
# --------------------------------
def _get_cuts() -> list[str]:
    l_cut = Data.conf['selection_ee'] if Data.particle == 'e' else Data.conf['selection']
    l_cut = [ cut.replace('PARTICLE', Data.particle) for cut in l_cut ]
    l_cut = [ cut.replace('Pi_'     ,         'pi_') for cut in l_cut ]

    log.debug('Using cuts:')
    for cut in l_cut:
        log.debug(cut)

    return l_cut
# --------------------------------
def _get_pid_cuts() -> str:
    l_cut = Data.conf['particles'][Data.particle]['pid_cuts']
    l_cut = [ f'({cut})' for cut in l_cut ]

    cut   = ' & '.join(l_cut)

    log.debug(f'Using PID cut: {cut}')

    return cut
# --------------------------------
def _get_polarity() -> str:
    if Data.sample in ['b1', 'b2', 'b3', 'b5', 'b8']:
        polarity = 'up'
    elif Data.sample in ['b4', 'b6', 'b7']:
        polarity = 'down'
    else:
        raise NotImplementedError(f'Invalid sample: {Data.sample}')

    log.debug(f'Using polarity: {polarity}')

    return polarity
# --------------------------------
def _get_config() -> dict[str,Any]:
    '''
    Returns
    --------------
    Returns configuration dictionary needed by PIDCalib2
    `make_eff_hists` method
    '''
    conf = gut.load_conf(package='rx_pid_data', fpath='config/config.yaml')

    cfg : dict              = {}
    cfg['sample']           = conf['samples'][Data.sample]
    cfg['pid_cuts']         = [ _get_pid_cuts(conf=conf) ] # PIDCalib2 expects a list of cuts, we use one cut, make list of one element...
    cfg['cuts']             = _get_prior_cuts(conf=conf)
    cfg['binning_file']     = _get_binning_path(conf=conf)
    cfg['magnet']           = _get_polarity()
    cfg['particle']         = Data.particle
    cfg['output_dir']       = Data.out_dir
    cfg['max_files']        = Data.max_files
    cfg['verbose']          = Data.verbose
    cfg['local_dataframe']  = None
    cfg['file_list']        = None
    cfg['samples_file']     = None

    return cfg 
# --------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    if Data.verbose:
        LogStore.set_level('rx_pid:create_pid_maps', 10)

    cfg = _get_config()

    if Data.dry_run:
        return

    make_eff_hists(cfg)
# --------------------------------
if __name__ == '__main__':
    main()
