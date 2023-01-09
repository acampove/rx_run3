
import utils_noroot as utnr
import numpy
import ROOT
import os

from rk.dilep_reso import calculator as calc_reso
from rk.dilep_reso import plot_reso
from rk.boundaries import boundaries

#---------------------------------------------
class data:
    log     = utnr.getLogger(__name__)
    out_dir = None 
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
        del(d_par['nsg'])

    d_data_ref = { boundaries(bounds) : d_par for bounds, d_par in d_data.items()}

    return d_data_ref
#---------------------------------------------
def get_resolution(rdf, brem):
    arr_bin       = numpy.array([10**4.0, 10**4.2, 10**4.3, 10**4.4])#, 10**4.4, 10**4.5, 10**4.6, 10**4.7, 10**5 ])
    d_bin         = {}
    d_bin['p1']   = arr_bin.tolist()
    d_bin['p2']   = arr_bin.tolist()

    d_par         = get_pars(brem) if not rdf.is_mc else {}
    obj           = calc_reso(rdf, binning=d_bin, fit=True, d_par=d_par)
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
def main():
    rdf = get_data(mc=False)
    get_resolution(rdf, 0)
#---------------------------------------------
if __name__ == '__main__':
    main()

