import os
import re 
import argparse 

import pandas            as pnd
import utils_noroot      as utnr
import matplotlib.pyplot as plt
#-----------------------------------------
class data:
    log    = utnr.getLogger(__name__)
    l_trig = ['ETOS', 'GTIS']
    l_brem = ['0', '1', '2']
    l_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_par  = ['delta_m', 'mu_MC', 's_sigma']

    plot_dir = utnr.make_dir_path('plots/comparison/')
    l_vers   = None
#-----------------------------------------
def get_pars(vers, trig, brem, year):
    cal_dir = os.environ['CALDIR']
    d_par   = utnr.load_json(f'{cal_dir}/qsq/{vers}/{year}.json')
    regex   = f'{trig}\s([\w,_]+)\s{brem}\sgamma'
    d_res   = dict(d_par)

    for key in d_par:
        mtch=re.match(regex, key)
        if not mtch:
            del(d_res[key])
            continue

        val = d_res[key]
        var = mtch.group(1)

        del(d_res[key])
        d_res[var] = val

    d_res = dict(sorted(d_res.items()))

    return d_res
#-----------------------------------------
def get_rows(trig, brem, year):
    l_row = []
    for vers in data.l_vers:
        d_par=get_pars(vers, trig, brem, year)

        for var, val in d_par.items():
            row=[year, brem, trig, var, val, vers]
            l_row.append(row)

    return l_row
#-----------------------------------------
def get_df():
    df = pnd.DataFrame(columns=['year', 'brem', 'trig', 'par_name', 'par_val', 'version'])
    for trig in data.l_trig:
        for brem in data.l_brem:
            for year in data.l_year:
                l_row= get_rows(trig, brem, year)
                df   = utnr.add_rows_to_df(df, l_row)

    return df
#-----------------------------------------
def main():
    df = get_df()

    for year in data.l_year:
        for trig in data.l_trig:
            for par in data.l_par:
                df_f = df  [  df.year     == year]
                df_f = df_f[df_f.trig     == trig]
                df_f = df_f[df_f.par_name ==  par]

                ax = None
                for version, df_v in df_f.groupby('version'):
                    ax=df_v.plot(x='brem', y='par_val', ax=ax, label=version)

                plt_path = f'{data.plot_dir}/{year}_{trig}_{par}.png'
                data.log.visible(f'Saving to: {plt_path}')
                plt.title(f'{trig}; {year}')
                plt.ylabel(par)
                plt.savefig(plt_path)
                plt.close('all')
#-----------------------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to perform compare versions of q2 smearing factors')
    parser.add_argument('-v', '--versions' , nargs='+', help='Versions to compare')
    args = parser.parse_args()

    data.l_vers = args.versions
#-----------------------------------------
if __name__ == '__main__':
    get_args()
    main()

