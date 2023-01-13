import utils_noroot as utnr
import argparse
import ROOT
import numpy

from rk.boundaries import boundaries

#-------------------------------------
class data:
    out_dir = None
    version = None

    l_kind  = ['par', 'res']
    l_brem  = [ 0, 1, 2 ]
#----------------------------
def par_to_res(d_par):
    d_res = {}
    for sbin, d_fit_par in d_par.items():
        if 'inf' in sbin: 
            continue

        [sg, _] = d_fit_par['sg']

        d_res[sbin] = sg

    return d_res
#-------------------------------------
def calculate_ratio(d_dt_str, d_mc_str):
    d_rat_str = {}
    for key in d_dt_str.keys():
        val_dt = d_dt_str[key]
        val_mc = d_mc_str[key]

        d_rat_str[key] = val_dt / val_mc

    return d_rat_str
#-------------------------------------
def get_ratio(kind, brem):
    mc_path  = f'output/resolution/mc/json/{data.version}/{kind}_brem_{brem}.json'
    dt_path  = f'output/resolution/data/json/{data.version}/{kind}_brem_{brem}.json'

    d_mc_str = utnr.load_json(mc_path)
    d_dt_str = utnr.load_json(dt_path)

    if kind == 'par':
        d_mc_str = par_to_res(d_mc_str)
        d_dt_str = par_to_res(d_dt_str)
    else:
        d_mc_str = d_mc_str
        d_dt_str = d_dt_str

    d_rat_str = calculate_ratio(d_dt_str, d_mc_str)

    return d_rat_str
#-------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-v', '--vers', help='Version of fits', type=str, required=True) 
    args = parser.parse_args()

    data.version = args.vers
    data.out_dir = utnr.make_dir_path(f'output/resolution/ratio/{data.version}')
#-------------------------------------
def convert_to_hist(d_rat_str):
    s_bound = set()
    for sbound, _ in d_rat_str.items():
        bnd = boundaries(sbound)
        (p1, p2), _ = bnd.data
        s_bound.add(p1)
        s_bound.add(p2)

    l_bound = list(s_bound)
    nbin    = len(l_bound) - 1
    l_bound.sort()
    arr_bound = numpy.array(l_bound)

    hist = ROOT.TH2F('hist', '', nbin, arr_bound, nbin, arr_bound)

    for sbound, ratio in d_rat_str.items():
        bnd = boundaries(sbound)
        (p1, p2), (q1, q2) = bnd.data

        pval = 0.5 * (p1 + p2)
        qval = 0.5 * (q1 + q2)

        hist.Fill(pval, qval, ratio)

    return hist
#-------------------------------------
def main():
    get_args()
    ofile = ROOT.TFile(f'{data.out_dir}/file.root', 'recreate')
    for kind in data.l_kind:
        for brem in data.l_brem:
            d_rat_str = get_ratio(kind, brem)
            utnr.dump_json(d_rat_str, f'{data.out_dir}/{kind}_brem_{brem}.json')

            h_rat = convert_to_hist(d_rat_str)
            h_rat.SetName(f'h_{kind}_brem_{brem}')
            h_rat.Write()

    ofile.Close()
#-------------------------------------
if __name__ == '__main__':
    main()

