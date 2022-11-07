import utils_noroot as utnr
import os
import argparse
import ROOT

from rk.selection import selection as rksl
from rk.mva       import mva_man

#-------------------
class data:
    log     = utnr.getLogger(__name__)
    cal_dir = os.environ['CALDIR']
    dat_dir = os.environ['DATDIR']

    l_all_trig = ['ETOS', 'GTIS']
    l_all_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_all_brem = [0, 1, 2]
    dat_vers   = 'v10.11tf'
    fraction   = 0.1
    bdt_dir    = '/publicfs/ucas/user/campoverde/Data/RK/MVA/electron/bdt_v10.14.a0v2ss'
    b_mass     = 'B_const_mass_M[0]'
    j_mass     = 'Jpsi_M'

    version    = None
    l_trig     = None 
    l_year     = None 
    l_brem     = None 
#-------------------
def get_df(year, trig, is_data=None):
    if is_data not in [True, False]:
        data.log.error(f'Dataset type not specified')
        raise

    proc = 'data_ee' if is_data else 'ctrl_ee'
    utnr.make_dir_path('cached')
    cache_path = f'cached/{year}_{trig}_{proc}.root'
    if os.path.isfile(cache_path):
        data.log.visible(f'Found cached file: {cache_path}[tree]')
        rdf = ROOT.RDataFrame('tree', cache_path)
        return rdf

    data_path = f'{data.dat_dir}/{proc}/{data.dat_vers}/{year}.root'
    rdf = ROOT.RDataFrame('KEE', data_path)

    if data.fraction < 1.0:
        nentries = rdf.Count().GetValue()
        nkeep    = int(data.fraction * nentries)

        data.log.visible(f'Using {nkeep}/{nentries} entries')
        rdf = rdf.Range(nkeep)

    rdf = apply_selection(rdf, trig, year, proc)

    data.log.visible(f'Caching: {cache_path}[tree]')
    rdf.Snapshot('tree', cache_path)

    return rdf
#-------------------
def add_bdt(rdf, trig):
    rdf = rdf.Define('b_mass', data.b_mass)
    rdf = rdf.Define('j_mass', data.j_mass)
    rdf = rdf.Define('nbrem' , 'L1_BremMultiplicity + L2_BremMultiplicity')

    d_data        = rdf.AsNumpy(['b_mass', 'j_mass', 'nbrem'])
    man           = mva_man(rdf, data.bdt_dir, trig)
    d_data['BDT'] = man.get_scores()

    return ROOT.RDF.MakeNumpyDataFrame(d_data)
#-------------------
def apply_selection(rdf, trig, year, proc):
    d_cut = rksl('all_gorder', trig, year, proc, q2bin='jpsi')

    bdt_cut = d_cut['bdt']
    del(d_cut['bdt'])

    for key, cut in d_cut.items():
        rdf=rdf.Filter(cut, key)

    rdf = add_bdt(rdf, trig)
    rdf = rdf.Filter(bdt_cut, 'BDT')

    return rdf
#-------------------
def fit(df, fix=None):
    return {'delta_m' : 1, 'sigma_m' : 2, 'mu' : 3}
#-------------------
def get_table(trig=None, year=None, brem=None):
    df_sim    = get_df(year, trig, is_data=False)
    df_dat    = get_df(year, trig, is_data= True)

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
#-------------------

