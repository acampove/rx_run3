'''
This script is meant to make plots of the efficiency in MC
in function of the true q2
'''
import copy
import argparse
import pandas as pnd

from ROOT                  import RDataFrame
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
    d_full_sel['q2'] = '(1)'

    d_part_sel = copy.deepcopy(d_full_sel)
    d_part_sel['bdt'] = '(1)'

    part_sel = _string_from_dictionary(data=d_part_sel)
    full_sel = _string_from_dictionary(data=d_full_sel)

    rdf = rdf.Define(Data.pass_sel, full_sel)
    rdf = rdf.Define(Data.pass_all, part_sel)

    return rdf
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
    '''
    if Data.cfg is None:
        raise ValueError('Config not set')

    [sample, trigger] = Data.cfg.input[f'{Data.analysis}_{Data.channel}']

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = _add_flags(rdf=rdf, sample=sample, trigger=trigger)

    l_col = ['q2_true', 'pass_all', 'pass_sel']
    data  = rdf.AsNumpy(l_col)
    df    = pnd.DataFrame(data)

    return df
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()
    Data.cfg = gut.load_conf(package='rx_plotter_data', fpath='efficiency/vs_q2.yaml')

    df = _get_data()
    print(df)
    #_plot_efficiencies()
# ----------------------
if __name__ == '__main__':
    main()
# ----------------------
