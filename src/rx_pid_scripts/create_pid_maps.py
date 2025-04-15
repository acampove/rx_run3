'''
Script meant to be used to create PID maps
'''
# pylint: disable=line-too-long

import argparse
from importlib.resources      import files
from pidcalib2.make_eff_hists import make_eff_hists

import yaml

# --------------------------------
class Data:
    '''
    Data class
    '''
    l_particle : list[str] = ['e', 'Pi', 'K', 'Mu', 'P']

    cfg_vers: str
    bin_vers: str
    particle: str
    polarity: str
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
    parser.add_argument('-m', '--polarity', type=str, help='Polarity'     , choices=['up', 'down'] , required=True)
    parser.add_argument('-s', '--sample'  , type=str, help='Sample'       ,                          default='2024')
    parser.add_argument('-d', '--dry-run' ,           help='Enable dry-run mode (default: False)'  , action='store_true')
    # These are by default None and will be used as in PIDCalib2's make_eff_hists
    parser.add_argument('-M', '--maxfiles', type=str, help='Limit number of files to this value')
    parser.add_argument('-v', '--verbose' , help='Will print debug messages', action='store_true')

    args          = parser.parse_args()
    Data.cfg_vers = args.cfg_vers
    Data.bin_vers = args.bin_vers
    Data.particle = args.particle
    Data.polarity = args.polarity
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
def _initialize() -> None:
    Data.conf                 = _load_data(kind='config' )

    Data.conf['sample']       = Data.conf['samples'][Data.sample]
    del Data.conf['samples']

    Data.conf['pid_cuts']     = Data.conf['particles'][Data.particle]['pid_cuts']
    Data.conf['bin_vars']     = Data.conf['particles'][Data.particle]['bin_vars']
    del Data.conf['particles']

    Data.conf['magnet']       = Data.polarity
    Data.conf['particle']     = Data.particle
    Data.conf['cuts']         = None
    Data.conf['output_dir']   = 'pidcalib_output'
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

    make_eff_hists(Data.conf)
# --------------------------------
if __name__ == '__main__':
    main()
