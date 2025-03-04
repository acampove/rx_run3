'''
Script used to plot binned quantities
'''
import argparse
from importlib.resources import files

import yaml
import numpy
from ROOT                    import RDataFrame, EnableImplicitMT
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_2d import Plotter2D as Plotter
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection as sel

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
    d_cut = Data.cfg['cuts']
    d_sel.update(d_cut)

    for cut_name, cut_value in d_sel.items():
        log.info(f'{cut_name:<20}{cut_value}')
        rdf = rdf.Filter(cut_value, cut_name)

    #rep = rdf.Report()
    #rep.Print()

    return rdf
# -----------------------------
def _cuts_from_binning(l_name : list[str], l_setting : list, index : int) -> list[str]:
    var_name            = l_name[index]
    [nbins, minv, maxv] = l_setting[index]['binning']
    arr_bound           = numpy.linspace(minv, maxv, nbins)
    l_bound             = arr_bound.tolist()
    l_min               = l_bound[:-1]
    l_max               = l_bound[+1:]

    l_cut = [ f'({var_name} > {minv:>3.0f}) && ({var_name} <={maxv:>3.0f})' for minv, maxv in zip(l_min, l_max)]

    return l_cut
# -----------------------------
def _get_normalized_value(rdf : RDataFrame, l_cut : list[str]) -> dict[str,float]:
    var_name = Data.cfg['target']['variable']
    quantity = Data.cfg['target']['quantity']

    d_rdf = { cut : rdf.Filter(cut) for cut in l_cut }

    if quantity == 'RMS':
        d_rms = { cut : rdf.StdDev(var_name) for cut, rdf in d_rdf.items() }
        d_frq = { cut : rdf.Count()          for cut, rdf in d_rdf.items() }
    else:
        raise NotImplementedError(f'Invalid quantity: {quantity}')

    d_nrm = {}
    log.info('Evaluating nodes')
    for cut in d_rdf:
        rms = d_rms[cut]
        frq = d_frq[cut]

        rms_val = rms.GetValue()
        frq_val = frq.GetValue()

        if frq_val == 0:
            log.warning(f'Zero yield at: {cut}')
            d_nrm[cut] = numpy.nan
        else:
            nrm_rms    = rms_val / frq_val
            log.info(f'{nrm_rms:.3f}')
            d_nrm[cut] = nrm_rms

    return d_nrm
# -----------------------------
def _plot(rdf : RDataFrame) -> None:
    ptr=Plotter(rdf=rdf, cfg=Data.cfg)
    ptr.run()
# -----------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _load_config()

    d_axis = Data.cfg['axes']

    l_name = list(d_axis.keys())
    l_sett = list(d_axis.values())

    l_cutx = _cuts_from_binning(l_name, l_sett, 0)
    l_cuty = _cuts_from_binning(l_name, l_sett, 1)

    l_cutxy = []
    for cutx in l_cutx:
        for cuty in l_cuty:
            cutxy  = f'({cutx}) && ({cuty})'
            l_cutxy.append(cutxy)

    rdf     = _get_rdf()
    d_nvval = _get_normalized_value(rdf, l_cutxy)

    return
    rdf     = _attach_normalized_value(d_nval)

    _plot(rdf)
# -----------------------------
if __name__ == '__main__':
    main()
