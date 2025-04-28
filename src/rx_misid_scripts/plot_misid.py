'''
Script used to plot mass distributions associated to samples in data and MC
used to study fully hadronic mis-ID backgrounds
'''
import copy
import argparse
from importlib.resources import files

import yaml
import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt
from ROOT                    import RDataFrame, RDF
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D

log=LogStore.add_logger('rx_misid:plot_misid')
# ---------------------------------------
class Data:
    '''
    Data class
    '''
    file_path : str
    cfg       : dict

    plt.style.use(mplhep.style.LHCb2)
# ---------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script meant to make plots for the samples used to study fully hadronic misID')
    parser.add_argument('-p','--path', type=str, help='Path to input file holding dataframe', required=True)
    args = parser.parse_args()

    Data.file_path = args.path
# ---------------------------------------
def _load_conf() -> None:
    conf_path = files('rx_misid_data').joinpath('plots.yaml')
    with open(conf_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)
# ---------------------------------------
def _rdf_from_df(df : pnd.DataFrame) -> dict[str,RDataFrame]:
    df      = df.drop(columns=['kind', 'hadron', 'bmeson'])
    rdf_wgt = RDF.FromPandas(df)
    rdf_raw = rdf_wgt.Redefine('weight','1')

    return {'Weighted' : rdf_wgt, 'Unweighted' : rdf_raw}
# ---------------------------------------
def _get_conf(df : pnd.DataFrame, kind : str) -> dict:
    nentries = len(df)
    cfg = copy.deepcopy(Data.cfg)
    for var, d_plot in cfg['plots'].items():
        d_plot['title'] = f'{kind}; Entries={nentries}'
        d_plot['name' ] = f'{var}_{kind}' 

    return cfg
# ---------------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _load_conf()

    df    = pnd.read_parquet(Data.file_path)
    d_rdf = _rdf_from_df(df)
    cfg   = _get_conf(df)

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ---------------------------------------
if __name__ == '__main__':
    main()
