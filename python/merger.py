import argparse
import os

import utils_noroot as utnr

#----------------------------
class data:
    log    = utnr.getLogger(__name__)
    in_ver = None
    ot_ver = None
    caldir = os.environ['CALDIR']

    l_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_trig = ['ETOS', 'GTIS']
    l_brem = [0, 1, 2]
#----------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-i', '--input' , type=str, help='Input version')
    parser.add_argument('-o', '--output', type=str, help='Output version')
    args = parser.parse_args()

    data.in_ver = args.input
    data.ot_ver = args.output
#----------------------------
def get_map(year, trig, brem):
    sim_map_path = f'plots/fits/{data.in_ver}/sim_{trig}_{year}_{brem}.json'
    dat_map_path = f'plots/fits/{data.in_ver}/dat_{trig}_{year}_{brem}.json'

    d_sim_par    = utnr.load_json(sim_map_path)
    d_dat_par    = utnr.load_json(dat_map_path)

    delta_m      = float(d_dat_par['mu']) - float(d_sim_par['mu'])
    sigma_m      = float(d_dat_par['sg']) / float(d_sim_par['sg'])
    mu_MC        = float(d_sim_par['mu'])

    d_table   = {}

    d_table[f'{trig} delta_m {brem} gamma'] = delta_m
    d_table[f'{trig} s_sigma {brem} gamma'] = sigma_m
    d_table[f'{trig} mu_MC {brem} gamma'  ] = mu_MC

    return d_table
#----------------------------
def merge(year):
    d_map = {}

    for trig in data.l_trig:
        for brem in data.l_brem:
            d_tmp=get_map(year, trig, brem)
            d_map.update(d_tmp)

    map_dir  = utnr.make_dir_path(f'{data.caldir}/qsq/{data.ot_ver}')
    map_path = f'{map_dir}/{year}.json'

    data.log.visible(f'Saving to: {map_path}')
    utnr.dump_json(d_map, map_path)
#----------------------------
if __name__ == '__main__':
    get_args()
    [merge(year) for year in data.l_year]

