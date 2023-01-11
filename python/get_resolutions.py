import utils_noroot as utnr
import numpy
import ROOT
import os
import argparse

from rk.dilep_reso import calculator as calc_reso
from rk.boundaries import boundaries

#---------------------------------------------
class data:
    log     = utnr.getLogger(__name__)
    out_dir = None 
    brem    = None
    sim     = None
    l_ibin  = None
#---------------------------------------------
def get_data(mc=None, trig='ETOS', year='2018'):
    dat_dir = os.environ['DATDIR']

    if mc:
        data.out_dir = 'output/resolution/mc'
        file_path = f'cached/{year}_{trig}_ctrl_ee.root'
    else:
        data.out_dir = 'output/resolution/data'
        file_path = f'cached/{year}_{trig}_data_ee.root'

    rdf = ROOT.RDataFrame('tree', file_path)
    rdf.is_mc = mc

    return rdf
#---------------------------------------------
def get_pars(brem):
    out_dir   = 'output/resolution/mc'
    json_path = f'{out_dir}/json/par_brem_{brem}.json'

    d_data = utnr.load_json(json_path)
    for sbin, d_par in d_data.items():
        if 'inf' in sbin:
            continue

        del(d_par['mu'])
        del(d_par['sg'])

    d_data_ref = { boundaries(bounds) : d_par for bounds, d_par in d_data.items()}

    return d_data_ref
#---------------------------------------------
def get_pdf_name(brem):
    if   brem == 0:
        return 'dscb'
    elif brem == 1:
        return 'johnson'
    elif brem == 2:
        return 'johnson'
    else:
        data.log.error(f'Invalid brem: {brem}')
        raise
#---------------------------------------------
def get_resolution(rdf, brem):
    arr_bin       = numpy.array([0., 15000., 19000., 25000., 30000., 50000, 100000])
    d_bin         = {}
    d_bin['p1']   = arr_bin.tolist()
    d_bin['p2']   = arr_bin.tolist()

    d_par         = {} if rdf.is_mc else get_pars(brem)
    obj           = calc_reso(rdf, binning=d_bin, fit=True, d_par=d_par, signal=get_pdf_name(brem), l_ibin=data.l_ibin)
    obj.plot_dir  = f'{data.out_dir}/plots'
    d_res, d_par  = obj.get_resolution(brem=brem)

    dump_to_json(d_res, d_par, brem)
#---------------------------------------------
def dump_to_json(d_res, d_par, brem):
    d_res = { str(key) : val for key, val in d_res.items() }
    d_par = { str(key) : val for key, val in d_par.items() }
    
    utnr.dump_json(d_res, f'{data.out_dir}/json/res_brem_{brem}.json')
    utnr.dump_json(d_par, f'{data.out_dir}/json/par_brem_{brem}.json')
#---------------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to fit data and MC to extract resolution parameters for mee in bins of the electron momentum')
    parser.add_argument('-b', '--brem', type=int, help='Brem category' , choices=[0, 1, 2], required=True)
    parser.add_argument('-s', '--sim' , help='Will only do MC fit', action='store_true')
    parser.add_argument('-i', '--ibin', nargs='+', help='List of bins (integers) to fit', default=[])

    args = parser.parse_args()

    data.brem   = args.brem
    data.sim    = args.sim
    data.l_ibin = args.ibin
#---------------------------------------------
def main():
    get_args()

    rdf = get_data(mc=data.sim)
    get_resolution(rdf, data.brem)
#---------------------------------------------
if __name__ == '__main__':
    main()

