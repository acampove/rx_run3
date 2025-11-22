'''
Script used to plot PID distributions from:

    - Samples with PID removed and original ones
    - Check that the distributions agree when the former ones have had the PID added
'''

import os
import re
import copy
import glob
import argparse

import mplhep
import matplotlib.pyplot as plt

from ROOT                    import RDataFrame # type: ignore
from dmu.logging.log_store   import LogStore
from ap_utilities.decays     import utilities          as aput
from dmu.plotting.plotter_1d import Plotter1D          as Plotter
from dmu.generic             import utilities          as gut
from dmu.generic             import version_management as vmn
from omegaconf               import DictConfig
from rx_data.rdf_getter      import RDFGetter

log = LogStore.add_logger('rx_plots:validate_nopid')
# ----------------------------
class Data:
    '''
    Data class
    '''
    cfg     : dict
    weight  : str
    samples : dict[str,str] # Dictionary with sample -> trigger name
    regex   = r'mc_\w+_\d{8}_(.*)_(Hlt2.*MVA)_\w+\.root' # Needed to extract sample and trigger name

    plt.style.use(mplhep.style.LHCb2)
    ana_dir = os.environ['ANADIR']
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='')
    _ = parser.parse_args()
# ----------------------------
def _get_rdf(sample : str, trigger : str, has_pid : bool) -> RDataFrame:
    if not has_pid:
        trigger = f'{trigger}_noPID'

    weight = '1.0' if has_pid else Data.weight

    with RDFGetter.exclude_friends(names=[
        'brem_track_2',
        'hop',
        'mva',
        'swp_cascade',
        'swp_jpsi_misid']):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)
        rdf = rdf.Define('weights', weight)

    return rdf
# ----------------------------
def _compare(
        sample    : str,
        rdf_nopid : RDataFrame,
        rdf_yspid : RDataFrame,
        rdf_xcpid : RDataFrame) -> None:

    d_rdf={'Original'             : rdf_yspid,
           'New with PID'         : rdf_xcpid,
           f'{Data.weight} x New' : rdf_nopid}

    cfg   = _get_config_with_title(sample=sample)
    ptr   = Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ----------------------------
def _get_config_with_title(sample : str) -> dict:
    cfg  = copy.deepcopy(Data.cfg)

    for _, settings in cfg['plots'].items():
        settings['title'] = sample
        name = settings['name']
        settings['name'] = f'{name}_{sample}'

    return cfg
# ----------------------------
def _load_config(channel : str) -> None:
    cfg = gut.load_data(
            package='rx_plotter_data',
            fpath  =f'no_pid/{channel}.yaml')

    plt_dir = f'{Data.ana_dir}/plots/checks/validate_nopid/{channel}'
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
def _get_sample_trigger(fpath : str) -> tuple[str,str]:
    '''
    Parameters
    ---------------
    fpath : path to ROOT file

    Returns
    ---------------
    tuple with sample and HLT trigger name
    '''
    fname = os.path.basename(fpath)
    mtch  = re.match(Data.regex, fname)
    if mtch is None:
        raise ValueError(f'Cannot match with {Data.regex} in {fname}')

    sample, trigger = mtch.groups()
    sample = aput.name_from_lower_case(sample)
    log.debug(f'Found: {sample}/{trigger}')

    return sample, trigger
# ----------------------------
def _load_samples() -> None:
    '''
    This is meant to fill Data.samples
    '''
    fdir    = f'{Data.ana_dir}/Data/rk_nopid/main'
    dpath   = vmn.get_last_version(dir_path=fdir, version_only=False)
    file_wc = f'{dpath}/*.root'
    l_fpath = glob.glob(file_wc)
    if len(l_fpath) == 0:
        raise FileNotFoundError(f'No files found in: {file_wc}')

    l_sample_trigger = [ _get_sample_trigger(fpath=fpath) for fpath in l_fpath ]
    Data.samples     = dict(l_sample_trigger)
# ----------------------------
def main(cfg : DictConfig | None = None):
    '''
    Start here
    '''
    if cfg is None:
        _parse_args()

    _load_samples()

    for sample, trigger in Data.samples.items():
        if sample in ['Bu_JpsiK_ee_eq_DPC']:
            continue

        if 'btosllball' in sample:
            Data.weight  = '0.5' if 'MuMu' in trigger else '0.1'
        elif sample in ['Bu_JpsiPi_ee_eq_DPC']:
            Data.weight  = '0.5' if 'MuMu' in trigger else '2e-2'
        else:
            Data.weight  = '0.5' if 'MuMu' in trigger else '1e-4'

        channel      = 'mm'  if 'MuMu' in trigger else   'ee'
        _load_config(channel=channel)

        rdf_nopid = _get_rdf(sample=sample, trigger=trigger, has_pid=False)
        rdf_yspid = _get_rdf(sample=sample, trigger=trigger, has_pid=True )
        rdf_xcpid = _apply_pid(rdf_nopid)

        _compare(
            sample   =   sample,
            rdf_nopid=rdf_nopid,
            rdf_yspid=rdf_yspid,
            rdf_xcpid=rdf_xcpid)
# ----------------------------
if __name__ == '__main__':
    main()
