'''
This script is meant to make plots of the efficiency in MC
in function of the true q2
'''
import copy
import random
import argparse
from typing       import cast

import numpy
import mplhep
import matplotlib.pyplot   as plt
import pandas              as pnd
from hist                  import Hist
from hist.axis             import Regular
from ROOT                  import RDataFrame
from boost_histogram.axis  import Axis
from omegaconf             import DictConfig
from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

log=LogStore.add_logger('rx_plots:efficiency_vs_q2')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    cfg     : DictConfig|None = None
    channel : str|None        = None
    analysis: str|None        = None

    pass_all = 'pass_all'
    pass_sel = 'pass_sel'
    mplhep.style.use('LHCb2')
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script needed to plot efficiency vs true q2')
    parser.add_argument('-c', '--channel' , type=str, help='Channel' , choices=['ee', 'mm'])
    parser.add_argument('-a', '--analysis', type=str, help='Analysis', choices=['rk', 'rkst'])
    args = parser.parse_args()

    Data.channel  = args.channel
    Data.analysis = args.analysis
# ----------------------
def _string_from_dictionary(data :  dict[str,str]) -> str:
    '''
    Parameters
    -------------
    data: Dictionary with selection

    Returns
    -------------
    String with concatenated selection, to be used to define new columns in dataframe
    '''
    l_expr = list(data.values())
    l_expr = [f'({expr})' for expr in l_expr ]

    full_expr = ' && '.join(l_expr)
    full_expr = f'int({full_expr})'

    return full_expr
# ----------------------
def _add_flags(
    rdf     : RDataFrame,
    sample  : str,
    trigger : str) -> RDataFrame:
    '''
    Parameters
    -------------
    rdf    : ROOT dataframe corresponding to a given MC sample
    sample : Name of sample, e.g. DATA_24...
    trigger: HLT2 trigger name

    Returns
    -------------
    Same dataframe with:
    - pass_sel: True if passes selection but mva
    - pass_all: True if passes full selection
    '''
    d_full_sel = sel.selection(q2bin='jpsi', process=sample, trigger=trigger)
    d_full_sel['q2']   = '(1)'
    d_full_sel['mass'] = '(1)'

    d_part_sel = copy.deepcopy(d_full_sel)
    d_part_sel['bdt'] = '(1)'

    part_sel = _string_from_dictionary(data=d_part_sel)
    full_sel = _string_from_dictionary(data=d_full_sel)

    rdf = rdf.Define(Data.pass_sel, part_sel)
    rdf = rdf.Define(Data.pass_all, full_sel)

    return rdf
# ----------------------
def _get_mcdt_q2(sample : str, trigger : str) -> numpy.ndarray:
    '''
    Parameters
    -------------
    sample: MC sample, e.g. mc_bdksee...
    trigger: HLT2 trigger

    Returns
    -------------
    Numpy array with q2 values from MCDT divided by 1000_000
    '''
    gtr    = RDFGetter(sample=sample, trigger=trigger, tree='MCDecayTree')
    rdf    = gtr.get_rdf()
    rdf    = cast(RDataFrame, rdf)
    arr_q2 = rdf.AsNumpy(['q2'])['q2']

    return arr_q2
# ----------------------
def _get_data() -> pnd.DataFrame:
    '''
    Parameters
    -------------
    None

    Returns
    -------------
    Pandas DataFrame with:
        - True q2
        - Selection flag, true for entries passing selection except for MVA
        - MVA flag, true for entries passing selection, including MVA
        - Array of true q2 attached as an atribute with key "total"
    '''
    if Data.cfg is None:
        raise ValueError('Config not set')

    log.debug('Loading data')

    [sample, trigger] = Data.cfg.input[f'{Data.analysis}_{Data.channel}']

    with RDFGetter.multithreading(nthreads=5):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf()
        rdf = _add_flags(rdf=rdf, sample=sample, trigger=trigger)

        l_col = ['q2_true', 'pass_all', 'pass_sel']
        data  = rdf.AsNumpy(l_col)

    df    = pnd.DataFrame(data)
    df.attrs['total'] = _get_mcdt_q2(sample=sample, trigger=trigger)

    return df
# ----------------------
def _get_hist(df : pnd.DataFrame, axis : Axis, flag : str) -> Hist:
    '''
    Parameters
    -------------
    df : DataFrame with q2_true column and flags

    Returns
    -------------
    Histogram with q2 distribution
    '''
    if flag == 'total':
        arr_q2 = df.attrs['total']
    else:
        df_filt = df[df[flag] == 1]
        df_filt = cast(pnd.DataFrame, df_filt)
        arr_q2  = df_filt['q2_true'].to_numpy()

    arr_q2    = arr_q2 / 1000_000.
    histogram = Hist(axis)
    histogram.fill(q2=arr_q2)

    return histogram
# ----------------------
def _check_none(obj : DictConfig|None, kind : str) -> DictConfig:
    '''
    Parameters
    -------------
    obj : omegaconf dictionary with configuration
    kind: Key that has to exist in config

    Returns
    -------------
    The config if it passes checks
    '''
    if obj is None:
        raise ValueError(f'Config {kind} not found')

    if kind not in obj:
        raise ValueError(f'Cannot find key {kind} in config')

    return obj
# ----------------------
def _add_lines() -> None:
    '''
    Parameters
    -------------
    None

    Returns
    -------------
    None
    '''
    if 'lines' not in Data.cfg:
        log.warning('Not adding lines')
        return

    cfg   = _check_none(obj=Data.cfg, kind='lines')
    color = cfg.lines.styling.color
    style = cfg.lines.styling.style

    for bound in cfg.lines.bounds.values():
        plt.axvline(x=bound, color=color, linestyle=style)
# ----------------------
def _plot_efficiencies(df : pnd.DataFrame) -> None:
    '''
    Parameters
    -------------
    df: Pandas dataframe with: q2_true, pass_all and pass_sel
    with numpy array with q2 from MCDecayTree attached with key `total`

    Returns
    -------------
    Nothing, this should only plot efficiencies
    '''
    cfg       = _check_none(obj=Data.cfg, kind='hist_conf')
    hist_conf = cfg.hist_conf

    axis  = Regular(**hist_conf)

    h_all = _get_hist(df=df, axis=axis, flag='pass_all')
    h_sel = _get_hist(df=df, axis=axis, flag='pass_sel')
    h_den = _get_hist(df=df, axis=axis, flag='total'   )

    ran   = random.random()

    h_eff_sel = ran * h_sel / h_den
    h_eff_tot = ran * h_all / h_den

    h_eff_sel.plot(color='blue' , histtype='fill', alpha=0.3, label='No MVA')
    h_eff_tot.plot(color='red'  , histtype='step', label='Full efficiency')
    _add_lines()
    plt.ylabel(r'$\varepsilon \cdot RDM$')
    plt.legend()
    plt.show()
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    log.debug('Loading configuration')
    Data.cfg = gut.load_conf(package='rx_plotter_data', fpath='efficiency/vs_q2.yaml')

    df = _get_data()
    _plot_efficiencies(df=df)
# ----------------------
if __name__ == '__main__':
    main()
# ----------------------
