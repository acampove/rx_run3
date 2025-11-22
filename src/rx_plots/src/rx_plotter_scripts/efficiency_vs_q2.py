'''
This script is meant to make plots of the efficiency in MC
in function of the true q2
'''
import os
import copy
import random
import argparse
import formulate
from typing       import cast

import numpy
import mplhep
import matplotlib.pyplot   as plt
import pandas              as pnd
from hist                  import Hist
from hist.axis             import Regular
from ROOT                  import RDataFrame, RDF # type: ignore
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
    cfg     : DictConfig
    channel : str|None        = None
    analysis: str|None        = None

    pass_all = 'pass_all'
    pass_sel = 'pass_sel'
    mplhep.style.use('LHCb2')

    l_col    = ['q2', 'q2_true', 'pass_all', 'pass_sel']
    ana_dir  = os.environ['ANADIR']
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
    rdf     : RDF.RNode,
    sample  : str,
    trigger : str) -> RDF.RNode:
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
    rdf    = gtr.get_rdf(per_file=False)
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

    log.debug('Loading data')

    [sample, trigger] = Data.cfg.input[f'{Data.analysis}_{Data.channel}']

    with RDFGetter.multithreading(nthreads=5):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)
        rdf = _add_flags(rdf=rdf, sample=sample, trigger=trigger)
        data= rdf.AsNumpy(Data.l_col)

    df= pnd.DataFrame(data)
    df.attrs['total'] = _get_mcdt_q2(sample=sample, trigger=trigger)

    return df
# ----------------------
def _get_hist(
        df   : pnd.DataFrame,
        axis : Regular,
        var  : str,
        flag : str) -> Hist:
    '''
    Parameters
    -------------
    df  : DataFrame with q2_true column and flags
    axis: Hist's X axis, needed to make histogram
    var : q2 name, for numerator histogram, q2 or q2_true
    flag: Either 'total', 'pass_sel' or 'pass_all'.
          Needed to control what histogram is made

    Returns
    -------------
    Histogram with q2 distribution
    '''
    if flag == 'total':
        arr_q2 = df.attrs['total']
    else:
        df_filt = df[df[flag] == 1]
        df_filt = cast(pnd.DataFrame, df_filt)
        arr_q2  = df_filt[var].to_numpy()

    arr_q2    = arr_q2 / 1000_000.
    histogram = Hist(axis)
    histogram.fill(arr_q2)

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
    for bound in cfg.lines.bounds.values():
        plt.axvline(x=bound, **cfg.lines.styling)
# ----------------------
def _plot_efficiencies(
        df : pnd.DataFrame,
        var: str) -> None:
    '''
    Parameters
    -------------
    df : Pandas dataframe with: q2_true, pass_all and pass_sel
        with numpy array with q2 from MCDecayTree attached with key `total`
    var: Name of the q2 variable used to parametrize the efficiency

    Returns
    -------------
    Nothing, this should only plot efficiencies
    '''
    cfg       = _check_none(obj=Data.cfg, kind='hist_conf')
    hist_conf = cfg.hist_conf

    axis  = Regular(**hist_conf[var])

    h_all = _get_hist(df=df, axis=axis, flag='pass_all', var=var)
    h_sel = _get_hist(df=df, axis=axis, flag='pass_sel', var=var)
    h_den = _get_hist(df=df, axis=axis, flag='total'   , var=var)
    ran   = random.random()

    h_eff_sel = ran * h_sel / h_den
    h_eff_tot = ran * h_all / h_den

    h_eff_sel.plot(color='blue' , histtype='fill', alpha=0.3, label='No MVA')
    h_eff_tot.plot(color='red'  , histtype='step', label='Full selection')
    _add_lines()

    cfg = _check_none(obj=Data.cfg, kind='input')
    [sample, trigger] = cfg.input[f'{Data.analysis}_{Data.channel}']

    plt.title(f'{sample}; {trigger}')
    plt.ylabel('A.U.')
    plt.legend()
# ----------------------
def _get_out_path(var : str, q2bin : str) -> str:
    '''
    Parameters
    -------------
    var  : Name of variable in function of which efficiency is measured
    q2bin: E.g. central

    Returns
    -------------
    Path to PNG file where plot should go
    '''
    out_dir = f'{Data.ana_dir}/plots/checks/efficiency_vs_q2/{Data.analysis}_{Data.channel}/{q2bin}'
    os.makedirs(out_dir, exist_ok=True)

    out_path = f'{out_dir}/{var}.png'

    return out_path
# ----------------------
def _select_dataframe(df : pnd.DataFrame, q2bin : str) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    df    : Pandas dataframe
    q2bin : E.g. low, central...

    Returns
    -------------
    Dataframe after cut was applied
    '''
    if q2bin == 'none':
        log.debug('Skipping q2 cut')
        return df

    d_sel      = sel.load_selection_config()
    q2cut :str = d_sel['q2_common'][q2bin]
    cut        = formulate.from_root(string=q2cut)
    num_cut    = cut.to_numexpr() # type: ignore
    df         = df.query(num_cut)

    return df
# ----------------------
def _plot(
    df   : pnd.DataFrame,
    var  : str,
    q2bin: str) -> None:
    '''
    Parameters
    -------------
    df   : Pandas dataframe with q2 and MVA scores
    var  : Name of q2 variable to use for plots
    q2bin: E.g. central, high
    '''

    df = _select_dataframe(df=df, q2bin=q2bin)

    _plot_efficiencies(df=df, var=var)
    out_path = _get_out_path(var=var, q2bin=q2bin)

    log.info(f'Saving to: {out_path}')
    plt.savefig(out_path)
    plt.close()
# ----------------------
def _set_logs() -> None:
    '''
    Will set the log level of different tools
    '''
    LogStore.set_level('rx_selection:truth_matching', 30)
    LogStore.set_level('rx_selection:selection'     , 30)
    LogStore.set_level('rx_data:rdf_getter'         , 30)
# ----------------------
def main(cfg : DictConfig | None = None) -> None:
    '''
    Parameters
    ------------------
    cfg: Config storing settings if used as module instead of script 
    '''
    if cfg is None:
        _parse_args()
    else:
        Data.analysis = cfg.analysis
        Data.channel  = cfg.channel

    _set_logs()
    log.debug('Loading configuration')
    Data.cfg = gut.load_conf(package='rx_plotter_data', fpath='efficiency/vs_q2.yaml')

    df = _get_data()
    for q2bin in ['none', 'low', 'cen_low', 'cen_high', 'central', 'jpsi', 'psi2', 'high']:
        _plot(df=df, var='q2'     , q2bin=q2bin)
        _plot(df=df, var='q2_true', q2bin=q2bin)
# ----------------------
if __name__ == '__main__':
    main()
