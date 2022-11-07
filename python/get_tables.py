import utils_noroot as utnr
import os
import argparse

#-------------------
class data:
    log     = utnr.getLogger(__name__)
    cal_dir = os.environ['CALDIR']

    l_all_trig = ['ETOS', 'GTIS']
    l_all_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_all_brem = [0, 1, 2]

    version    = None
    l_trig     = None 
    l_year     = None 
    l_brem     = None 
#-------------------
def get_df(data=None):
    return None
#-------------------
def fit(df, fix=None):
    return {'delta_m' : 1, 'sigma_m' : 2, 'mu' : 3}
#-------------------
def get_table(trig=None, year=None, brem=None):
    df_sim    = get_df(data=False)
    df_dat    = get_df(data=True)

    d_sim_par = fit(df_sim, fix=     None)
    d_dat_par = fit(df_dat, fix=d_sim_par)

    delta_m = d_dat_par['delta_m']
    sigma_m = d_dat_par['sigma_m']
    mu_MC   = d_sim_par['mu']

    d_table = {}

    d_table[f'{trig} delta_m {brem} gamma'] = delta_m
    d_table[f'{trig} s_sigma {brem} gamma'] = sigma_m
    d_table[f'{trig} mu_MC {brem} gamma'  ] = mu_MC 

    return d_table
#-------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to produce q2 smearing factors systematic tables')
    parser.add_argument('-v', '--vers' , type =str, help='Version for output maps', required=True)
    parser.add_argument('-t', '--trig' , nargs='+', help='Triggers'       , choices=data.l_all_trig, default=data.l_all_trig)
    parser.add_argument('-y', '--year' , nargs='+', help='Years'          , choices=data.l_all_year, default=data.l_all_year)
    parser.add_argument('-b', '--brem' , nargs='+', help='Brem categories', choices=data.l_all_brem, default=data.l_all_brem)
    args = parser.parse_args()

    data.version= args.vers
    data.l_trig = args.trig
    data.l_year = args.year
    data.l_brem = args.brem
#-------------------
if __name__ == '__main__':
    get_args()
    for year in data.l_year:
        d_table = {}
        for trig in data.l_trig:
            for brem in data.l_brem:
                d_scale = get_table(trig=trig, year=year, brem=brem)
                d_table.update(d_scale)

        map_path = f'{data.cal_dir}/qsq/{data.version}/{year}.json'
        data.log.visible(f'Saving to: {map_path}')
        utnr.dump_json(d_table, map_path)

