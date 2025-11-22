#!/usr/bin/env python3

import os
import ROOT
import zfit
import numpy
import argparse
import utils_noroot       as utnr
import version_management as vmg

from logzero          import logger     as log 
from rk.dilep_reso    import calculator as calc_reso
from rk.boundaries    import boundaries
from q2_syst.data_set import data_set

#---------------------------------------------
class data:
    log     = utnr.getLogger(__name__)
    qsqsys  = os.environ['QSQSYS']
    cas_dir = os.environ['CASDIR']
    l_brem  = ['0', '1', '2'] 
    l_trig  = ['ETOS', 'GTIS']

    dset    = None
    vers    = None
    sim     = None
    l_ibin  = None
    out_dir = None
#---------------------------------------------
def delete_all_pars():
    d_par = zfit.Parameter._existing_params
    l_key = list(d_par.keys())

    for key in l_key:
        del(d_par[key])
#---------------------------------------------
def get_pars(brem, trig):
    out_dir   = f'{data.qsqsys}/get_q2_resolutions/mc'
    json_path = f'{out_dir}/json/{data.vers}/{trig}_{data.dset}/par_brem_{brem}.json'

    d_data = utnr.load_json(json_path)
    for sbin, d_par in d_data.items():
        if 'inf' in sbin:
            continue

        if d_par == {}:
            d_par = None
        else:
            del(d_par['mu'])
            del(d_par['sg'])

    d_data_ref = { boundaries(bounds) : d_par for bounds, d_par in d_data.items()}

    return d_data_ref
#---------------------------------------------
def get_pdf_name(brem):
    if   brem == '0':
        return 'dscb'
    elif brem == '1':
        return 'johnson'
    elif brem == '2':
        return 'johnson'
    else:
        data.log.error(f'Invalid brem: {brem}')
        raise
#---------------------------------------------
def get_binning(brem, trigger=None):
    if trigger == 'ETOS':
        arr_bin = numpy.array([0., 15000., 19000., 25000., 30000., 50000, 100000])
    else:
        arr_bin = numpy.array([0., 11000., 15000., 21000., 30000, 100000])

    return arr_bin
#---------------------------------------------
def get_resolution(rdf, brem, trig):
    arr_bin       = get_binning(brem, trigger=trig)

    d_bin         = {}
    d_bin['p1']   = arr_bin.tolist()
    d_bin['p2']   = arr_bin.tolist()

    d_par         = {} if rdf.is_mc else get_pars(brem, trig)
    obj           = calc_reso(rdf, binning=d_bin, fit=True, d_par=d_par, signal=get_pdf_name(brem), l_ibin=data.l_ibin)
    obj.plot_dir  = f'{data.out_dir}/plots/{data.vers}/{trig}_{data.dset}'
    d_res, d_par  = obj.get_resolution(brem=int(brem))

    dump_to_json(d_res, d_par, brem, trig)
#---------------------------------------------
def dump_to_json(d_res, d_par, brem, trig):
    d_res = { str(key) : val for key, val in d_res.items() }
    d_par = { str(key) : val for key, val in d_par.items() }

    res_path = f'{data.out_dir}/json/{data.vers}/{trig}_{data.dset}/res_brem_{brem}.json'
    par_path = f'{data.out_dir}/json/{data.vers}/{trig}_{data.dset}/par_brem_{brem}.json'
    if data.l_ibin == []:
        data.log.info('Saving to JSON information for all bins')
        utnr.dump_json(d_res, res_path)
        utnr.dump_json(d_par, par_path)
    else:
        data.log.info(f'Updating parameters for bins: {data.l_ibin}')
        d_res_old = utnr.load_json(res_path)
        d_par_old = utnr.load_json(par_path)

        d_res_old.update(d_res)
        d_par_old.update(d_par)

        utnr.dump_json(d_res_old, res_path)
        utnr.dump_json(d_par_old, par_path)
#---------------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to fit data and MC to extract resolution parameters for mee in bins of the electron momentum')
    parser.add_argument('-d', '--dset', type =str, help='Dataset'       , required=True, choices= ['r1', 'r2p1', '2017', '2018'])
    parser.add_argument('-t', '--trig', nargs='+', help='Trigger'       , default=data.l_trig, choices= data.l_trig) 
    parser.add_argument('-b', '--brem', nargs='+', help='Brem category' , default=data.l_brem, choices= data.l_brem)
    parser.add_argument('-v', '--vers', type =str, help='Output version', required=True)
    parser.add_argument('-s', '--sim' , help ='Will only do MC fit'     , action='store_true')
    parser.add_argument('-i', '--ibin', nargs='+', help='List of bins (integers) to fit', default=[])

    args = parser.parse_args()

    data.l_brem = args.brem
    data.l_trig = args.trig
    data.vers   = args.vers
    data.dset   = args.dset
    data.sim    = args.sim
    data.l_ibin = args.ibin
    data.out_dir= f'{data.qsqsys}/get_q2_resolutions/mc' if data.sim else f'{data.qsqsys}/get_q2_resolutions/data'
#---------------------------------------------
def save_binning(brem):
    arr_bin_tos = get_binning(brem, trigger='ETOS')
    arr_bin_tis = get_binning(brem, trigger='GTIS')

    l_bin_tos = arr_bin_tos.tolist()
    l_bin_tis = arr_bin_tis.tolist()

    l_bin_tos = [ str(ibin) for ibin in l_bin_tos ]
    l_bin_tis = [ str(ibin) for ibin in l_bin_tis ]

    bin_tos_str = ', '.join(l_bin_tos)
    bin_tis_str = ', '.join(l_bin_tis)

    line = '\\begin{tabular}{l l}\n'
    line+= '\\toprule\n'
    line+= 'Trigger & Binning      \\\\\n'
    line+= '\\midrule\n'
    line+=f'eTOS    & {bin_tos_str}\\\\\n'
    line+=f'gTIS!   & {bin_tis_str}\\\\\n'
    line+= '\\bottomrule\n'
    line+= '\\end{tabular}'

    tex_dir  = utnr.make_dir_path(f'{data.out_dir}/plots/{data.vers}')
    tex_path = f'{tex_dir}/binning.tex'
    log.info(f'Saving to: {tex_path}')
    with open(tex_path, 'w') as ofile:
        ofile.write(line)
#---------------------------------------------
def main():
    get_args()
    for brem in data.l_brem:
        save_binning(brem)
        for trig in data.l_trig:
            log.info(f'Running for brem/trig: {brem}/{trig}')

            odf = data_set(is_mc=data.sim, trigger=trig, dset=data.dset)
            odf.plt_dir =  f'{data.out_dir}/plots/{data.vers}/{trig}_{data.dset}/cal_wgt' 
            rdf = odf.get_rdf()
            rdf.is_mc = data.sim

            get_resolution(rdf, brem, trig)
            delete_all_pars()
#---------------------------------------------
if __name__ == '__main__':
    main()

