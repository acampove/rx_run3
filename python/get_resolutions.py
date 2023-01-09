
import utils_noroot as utnr
import numpy
import ROOT
import os

from rk.dilep_reso import calculator as calc_reso
from rk.dilep_reso import plot_reso

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

    return rdf
#---------------------------------------------
def get_resolution(rdf, brem):
    arr_bin       = numpy.array([10**4.0, 10**4.2, 10**4.3, 10**4.4])#, 10**4.4, 10**4.5, 10**4.6, 10**4.7, 10**5 ])
    d_bin         = {}
    d_bin['p1']   = arr_bin.tolist()
    d_bin['p2']   = arr_bin.tolist()
    
    obj           = calc_reso(rdf, binning=d_bin, fit=True, d_par={})
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
    rdf = get_data(mc=True)
    get_resolution(rdf, 0)
#---------------------------------------------
if __name__ == '__main__':
    main()

