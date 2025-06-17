'''
Script used to plot PID distributions from:

    - Samples with PID removed and original ones
    - Check that the distributions agree when the former ones have had the PID added
'''

import os
import copy
import glob
import argparse

import mplhep
import matplotlib.pyplot as plt
from ROOT                    import RDataFrame
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D as Plotter
from dmu.generic             import utilities as gut

log = LogStore.add_logger('rx_plots:validate_nopid')
# ----------------------------
class Data:
    '''
    Data class
    '''
    cfg     : dict
    channel : str

    plt.style.use(mplhep.style.LHCb2)
    ana_dir = os.environ['ANADIR']
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-c', '--channel', type=str, help='Channel where the validation will be ran', choices=['ee', 'mm'], required=True)
    args = parser.parse_args()

    Data.channel = args.channel
# ----------------------------
def _get_rdf(root_path : str, has_pid : bool) -> RDataFrame:
    if   Data.channel == 'ee':
        tname = 'Hlt2RD_BuToKpEE_MVA'
    elif Data.channel == 'mm':
        tname = 'Hlt2RD_BuToKpMuMu_MVA'
    else:
        raise ValueError(f'Invalid channel: {Data.channel}')

    if has_pid:
        tname = f'{tname}'
    else:
        tname = f'{tname}_noPID'

    wgt = '1.0' if has_pid else '0.1'

    rdf = RDataFrame(f'{tname}/DecayTree', root_path)
    rdf = rdf.Define('weights', wgt)

    return rdf
# ----------------------------
# ----------------------------
def _compare(
        root_path : str,
        rdf_nopid : RDataFrame,
        rdf_yspid : RDataFrame,
        rdf_xcpid : RDataFrame) -> None:

    d_rdf={'Original'     : rdf_yspid,
           'New with PID' : rdf_xcpid,
           '0.1 x New'    : rdf_nopid}

    fname = os.path.basename(root_path)
    cfg   = _get_config_with_title(fname=fname)
    ptr   = Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ----------------------------
def _get_config_with_title(fname : str) -> dict:
    title= fname.replace('.root', '')
    cfg  = copy.deepcopy(Data.cfg)

    for _, settings in cfg['plots'].items():
        settings['title'] = title
        name = settings['name']
        settings['name'] = f'{name}_{title}'

    return cfg
# ----------------------------
def _load_config() -> None:
    cfg = gut.load_data(
            package='rx_plotter_data',
            fpath  =f'no_pid/{Data.channel}.yaml')

    plt_dir = f'{Data.ana_dir}/plots/no_pid/{Data.channel}'

    cfg['saving'] = {'plt_dir' : plt_dir}

    Data.cfg = cfg
# ----------------------------
def _apply_pid(rdf : RDataFrame) -> RDataFrame:
    d_cut = Data.cfg['selection']['cuts']

    for name, expr in d_cut.items():
        rdf = rdf.Filter(expr, name)

    rdf = rdf.Redefine('weights', '1.0')

    del Data.cfg['selection']['cuts']

    return rdf
# ----------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    path_wc = f'{Data.ana_dir}/ana_prod/ntuples/no_pid/{Data.channel}/*.root'
    for root_path in glob.glob(path_wc):
        _load_config()
        log.info(root_path)

        rdf_nopid = _get_rdf(root_path=root_path, has_pid=False)
        rdf_yspid = _get_rdf(root_path=root_path, has_pid=True )
        rdf_xcpid = _apply_pid(rdf=rdf_nopid)

        _compare(
                root_path=root_path,
                rdf_nopid=rdf_nopid,
                rdf_yspid=rdf_yspid,
                rdf_xcpid=rdf_xcpid)
# ----------------------------
if __name__ == '__main__':
    main()
