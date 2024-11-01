'''
Script meant to do truth matching checks
'''
import os
import argparse

import yaml

from ROOT import RDataFrame

from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D as Plotter

log=LogStore.add_logger('dmu:physics:check_truth')
# ----------------------------------
def _set_logs() -> None:
    LogStore.set_level('dmu:plotting:Plotter'  , 30)
    LogStore.set_level('dmu:plotting:Plotter1D', 30)
# ----------------------------------
def _get_args() -> argparse.Namespace:
    '''
    Parse args
    '''
    parser = argparse.ArgumentParser(description='Script used to carry out checks on truth matching mechanisms for MC')
    parser.add_argument('-c', '--conf' , type=str, help='Path to config file', required=True)
    args = parser.parse_args()

    return args
# ----------------------------------
def _get_config(args : argparse.Namespace) -> dict:
    path = args.conf
    if not os.path.isfile(path):
        raise FileNotFoundError(f'Cannot find {path}')

    with open(path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# ----------------------------------
def _get_rdf(file_path : str, tree_path : str) -> RDataFrame:
    log.debug(f'Picking inputs from: {file_path}/{tree_path}')
    rdf = RDataFrame(tree_path, file_path)

    nentries = rdf.Count().GetValue()
    log.debug(f'Found {nentries} entries')

    return rdf
# ----------------------------------
def _preprocess_rdf(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    if 'max_entries' in cfg:
        max_entries = cfg['max_entries']
        rdf = rdf.Range(max_entries)

    return rdf
# ----------------------------------
def _check(cfg : dict) -> None:
    log.info(110 * '-')
    log.info(f'{"Sample":<20}{"Method":<20}{"Initial":<15}{"":<15}{"Final":<15}{"":15}{"Efficiency":<10}')
    log.info(110 * '-')

    for sample_name in cfg['samples']:
        d_rdf     = {}
        file_path = cfg['samples'][sample_name]['file_path']
        tree_path = cfg['samples'][sample_name]['tree_path']
        rdf = _get_rdf(file_path, tree_path)
        rdf = _preprocess_rdf(rdf, cfg)

        for name, cut in cfg['samples'][sample_name]['methods'].items():
            rdf_truth = _check_kind(rdf, sample_name, name, cut)
            d_rdf[name] = rdf_truth
        log.info('')

        cfg_plt = cfg['samples'][sample_name]['plot']
        ptr=Plotter(d_rdf=d_rdf, cfg=cfg_plt)
        ptr.run()
# ----------------------------------
def _check_kind(rdf : RDataFrame, sample : str, name : str, cut : str) -> RDataFrame:
    nini = rdf.Count().GetValue()
    rdf  = rdf.Filter(cut, name)
    nfnl = rdf.Count().GetValue()
    eff  = nfnl / nini * 100

    log.info(f'{sample:<20}{name:<20}{nini:<15}{"":<15}{nfnl:<15}{"-->":15}{eff:10.2f}')

    return rdf
# ----------------------------------
def main():
    '''
    Script starts here
    '''
    _set_logs()
    args = _get_args()
    cfg  = _get_config(args)
    _check(cfg)
# ----------------------------------
if __name__ == '__main__':
    main()
