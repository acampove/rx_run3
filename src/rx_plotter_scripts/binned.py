'''
Script used to plot binned quantities
'''
import argparse
from importlib.resources import files

import yaml
from ROOT                  import RDataFrame, EnableImplicitMT
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

log=LogStore.add_logger('rx_plotter:binned')
# -----------------------------
class Data:
    '''
    data class
    '''
    conf_name : str
    cfg       : dict

    EnableImplicitMT(10)
# -----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Used to plot 2D distributions')
    parser.add_argument('-c', '--conf' , type=str, help='Name of config file')
    args = parser.parse_args()

    Data.conf_name = args.conf
# -----------------------------
def _load_config() -> None:
    conf_path = files('rx_plotter_data').joinpath(f'binned/{Data.conf_name}.yaml')
    with open(conf_path, encoding='utf-8') as ifile:
        Data.cfg = yaml.safe_load(ifile)
# -----------------------------
def _get_rdf() -> RDataFrame:
    q2bin   = Data.cfg['input']['q2bin']
    trigger = Data.cfg['input']['trigger']

    RDFGetter.samples = Data.cfg['input']['samples']
    gtr = RDFGetter(sample='DATA*', trigger=trigger)
    rdf = gtr.get_rdf()
    d_def = Data.cfg['define']
    for name, expr in d_def.items():
        rdf = rdf.Define(name, expr)

    d_sel = sel.selection(project='RK', analysis='EE', q2bin=q2bin, process='DATA')
    d_cut = Data.cfg['selection']
    d_sel.update(d_cut)

    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
        rdf = rdf.Filter(cut_value, cut_name)

    rep = rdf.Report()
    rep.Print()

    return rdf
# -----------------------------
def _plot(rdf : RDataFrame) -> None:
    pass
# -----------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _load_config()
    rdf = _get_rdf()

    _plot(rdf)
# -----------------------------
if __name__ == '__main__':
    main()
