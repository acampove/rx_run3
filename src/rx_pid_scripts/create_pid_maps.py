'''
Script meant to be used to create PID maps
'''
# pylint: disable=line-too-long

import argparse
from importlib.resources      import files

import yaml
from pidcalib2.make_eff_hists import make_eff_hists
from dmu.logging.log_store    import LogStore

log=LogStore.add_logger('rx_pid:create_pid_maps')
# --------------------------------
class Data:
    '''
    Data class
    '''
    l_particle : list[str] = ['e', 'Pi', 'K', 'Mu', 'P']

    out_dir : str
    cfg_vers: str
    bin_vers: str
    particle: str
    sample  : str
    conf    : dict
    dry_run : bool

    max_files: int
    verbose  : bool
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to calculate PID efficiencies using PIDCalib2')
    parser.add_argument('-c', '--cfg_vers', type=str, help='Version of configuration file'         , required=True)
    parser.add_argument('-b', '--bin_vers', type=str, help='Version of binning file'               , required=True)
    parser.add_argument('-p', '--particle', type=str, help='Particle name', choices=Data.l_particle, required=True)
    parser.add_argument('-s', '--sample'  , type=str, help='Sample'       ,                          required=True)
    parser.add_argument('-o', '--out_dir' , type=str, help='Directory where pkl files will go'     , required=True)
    parser.add_argument('-d', '--dry-run' ,           help='Enable dry-run mode (default: False)'  , action='store_true')
    # These are by default None and will be used as in PIDCalib2's make_eff_hists
    parser.add_argument('-M', '--maxfiles', type=int, help='Limit number of files to this value')
    parser.add_argument('-v', '--verbose' , help='Will print debug messages', action='store_true')

    args          = parser.parse_args()
    Data.out_dir  = args.out_dir
    Data.cfg_vers = args.cfg_vers
    Data.bin_vers = args.bin_vers
    Data.particle = args.particle
    Data.sample   = args.sample
    Data.dry_run  = args.dry_run
    Data.max_files= args.maxfiles
    Data.verbose  = args.verbose
# --------------------------------
def _get_bin_vars() -> str:
    if not hasattr(Data, 'conf'):
        raise AttributeError('Data class does not have a config dictionary')

    l_var   = Data.conf['bin_vars']
    var_str = '_'.join(l_var)

    return var_str
# --------------------------------
def _path_from_kind(kind : str) -> str:
    if kind == 'config':
        ext     = 'yaml'
        version = Data.cfg_vers
        path    = files('rx_pid_data').joinpath(f'{kind}/{version}.{ext}')
    elif kind == 'binning':
        ext     = 'json'
        version = Data.bin_vers
        bin_vars= _get_bin_vars()
        path    = files('rx_pid_data').joinpath(f'{kind}/{bin_vars}/{version}.{ext}')
    else:
        raise ValueError(f'Invalid kind: {kind}')

    return path
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
def _initialize() -> None:
    if Data.verbose:
        LogStore.set_level('rx_pid:create_pid_maps', 10)

    Data.conf                 = _load_data(kind='config')

    samples                   = Data.conf['samples_ee'] if Data.particle == 'e' else Data.conf['samples']
    Data.conf['sample']       = samples[Data.sample]
    del Data.conf['samples']
    del Data.conf['samples_ee']

    Data.conf['pid_cuts']     = Data.conf['particles'][Data.particle]['pid_cuts']
    Data.conf['bin_vars']     = Data.conf['particles'][Data.particle]['bin_vars']
    del Data.conf['particles']

    Data.conf['cuts']         = _get_cuts()
    del Data.conf['selection']

    Data.conf['magnet']       = Data.polarity
    Data.conf['particle']     = Data.particle
    Data.conf['output_dir']   = Data.out_dir 
    Data.conf['binning_file'] = _path_from_kind(kind='binning')
    Data.conf['max_files']       = Data.max_files
    Data.conf['verbose']         = Data.verbose
    Data.conf['local_dataframe'] = None
    Data.conf['file_list']       = None
    Data.conf['samples_file']    = None
# --------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    if Data.dry_run:
        return

    make_eff_hists(Data.conf)
# --------------------------------
if __name__ == '__main__':
    main()
