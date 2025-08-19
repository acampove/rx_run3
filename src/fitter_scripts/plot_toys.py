'''
Script used to plot distributions from toy fits
by loading pandas dataframes stored as parquet files
'''
import os
import argparse
from pathlib import Path
from typing  import Iterable

import pandas as pnd
from omegaconf             import DictConfig
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore
from fitter.toy_plotter    import ToyPlotter

log=LogStore.add_logger('fitter:plot_toys')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    log_lvl    : int
    source     : str
    version    : str
    identifier : str
    dry_run    : bool
    PARAM_WCARD= 'parameters_*.parquet'
# ----------------------
def _set_logs() -> None:
    '''
    This method will set the log level of this
    and other tools
    '''
    LogStore.set_level('fitter:plot_toys', Data.log_lvl)
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make plots from outputs of toy fits')
    parser.add_argument('-v', '--version'    , type=str, help='Version of toy fits, e.g. v2', required=True)
    parser.add_argument('-s', '--source'     , type=str, help='Identifier of place where parquet files are', required=True)
    parser.add_argument('-i', '--identifier' , type=str, help='Identifier of toy, if not passed, will do all toys')
    parser.add_argument('-l', '--log_level'  , type=int, help='Log level', default=20)
    parser.add_argument('-d', '--dry_run'    , help='If used, will skip plotting step', action='store_true')
    args = parser.parse_args()

    Data.version    = args.version
    Data.source     = args.source
    Data.identifier = args.identifier
    Data.log_lvl    = args.log_level
    Data.dry_run    = args.dry_run
# ----------------------
def _scandir_recursive(source_path : str, pattern='*.parquet')-> Iterable[str]:
    path = Path(source_path)
    for entry in os.scandir(path=path):
        if entry.is_dir(follow_symlinks=False):
            yield from _scandir_recursive(source_path=entry.path, pattern=pattern)
        elif entry.is_file() and entry.name.endswith(pattern[1:]):  # '*.parquet' -> 'parquet'
            yield entry.path
# ----------------------
def _get_dataframes(source_path : str) -> dict[str, pnd.DataFrame]:
    '''
    Parameters
    -------------
    source_path: Path where the search for PARAM_WCARD files are searched Returns
    -------------
    List of dataframes where each dataframe contains all the parameters
    for the toy fits to a given model
    '''
    log.debug('Looking for files in: {source_path}/**/{Data.PARAM_WCARD}')
    iterator = _scandir_recursive(source_path=source_path, pattern=Data.PARAM_WCARD)
    if hasattr(Data, 'identifier'):
        l_path = [ path for path in iterator if Data.identifier in path ]
    else:
        l_path = list(iterator)

    npath = len(l_path)
    if npath == 0:
        raise ValueError(f'No paths to {Data.PARAM_WCARD} found in {source_path}')

    log.info(f'Found {npath} dataframes')
    d_df = { path : pnd.read_parquet(path) for path in l_path }

    return d_df
# ----------------------
def _update_config(cfg : DictConfig, path : str) -> DictConfig:
    '''
    Parameters
    -------------
    cfg: Config as loaded from YAML
    path: Path to parquet file with dataframe data

    Returns
    -------------
    Config with values overriden:
    - Path to output has to be same as path to input, but in local machine
    '''
    source_path = cfg.paths[Data.source]
    target_path = os.environ['ANADIR']
    path        = path.replace(source_path, target_path)
    path        = f'{path}/{Data.version}'

    log.debug(f'Saving to: {path}')

    cfg.paths.saving.plt_dir = path

    return cfg
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    _set_logs()

    cfg         = gut.load_conf(package='fitter_data', fpath='toys/plotter.yaml')
    source_path = cfg.paths[Data.source]
    d_df        = _get_dataframes(source_path=source_path)

    log.info('Plotting:')
    for path, df in d_df.items():
        cfg = _update_config(cfg=cfg, path=path)
        if Data.dry_run:
            log.debug(f'    {path}')
            continue

        ptr = ToyPlotter(df=df, cfg=cfg)
        ptr.plot()
# ----------------------
if __name__ == '__main__':
    main()
