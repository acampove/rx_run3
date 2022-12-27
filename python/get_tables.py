import utils_noroot                as utnr
import matplotlib.pyplot           as plt
import os
import zfit
import numpy
import argparse
import ROOT

from zutils.plot  import plot      as zfp
from rk.selection import selection as rksl
from zutils.utils import result_to_latex 
from fitter       import zfitter
from rk.mva       import mva_man

#-------------------
class data:
    log      = utnr.getLogger(__name__)
    cal_dir  = os.environ['CALDIR']
    dat_dir  = os.environ['DATDIR']

    l_trig   = ['ETOS', 'GTIS']
    l_year   = ['2011', '2012', '2015', '2016', '2017', '2018', 'r1', 'r2p1']
    l_brem   = ['0', '1', '2']
    l_sys    = ['nom', 'nspd']

    dat_vers = 'v10.11tf'
    fraction = 1.0
    bdt_dir  = '/publicfs/ucas/user/campoverde/Data/RK/MVA/electron/bdt_v10.14.a0v2ss'
    b_mass   = 'B_const_mass_M[0]'
    j_mass   = 'Jpsi_M'

    trig     = None 
    year     = None 
    brem     = None 
    sim_only = None
    plt_dir  = None 

    obs      = zfit.Space('j_mass', limits=(2450, 3600))
    sig_pdf  = None
    bkg_pdf  = None

    mu       = zfit.Parameter('mu', 3060,  3040, 3100)
    sg       = zfit.Parameter('sg',   20,    10,  100)

    d_sig_ini        =   {}
    d_sig_ini['mu'  ]= 3060
    d_sig_ini['sg'  ]= 20.0
    d_sig_ini['ap_r']=  1.0
    d_sig_ini['pw_r']=  1.0
    d_sig_ini['ap_l']= -1.0
    d_sig_ini['pw_l']=  1.0
    d_sig_ini['ncbr']=  1000 
    d_sig_ini['ncbl']=  1000  
#-------------------
def get_years(dset):
    if   dset == 'r1':
        l_year = ['2011', '2012'] 
    elif dset == 'r2p1':
        l_year = ['2015', '2016'] 
    else:
        l_year = [dset] 

    return l_year
#-------------------
def get_cached_paths(dset, trig, proc):
    l_year = get_years(dset)
    l_path = [ f'cached/{year}_{trig}_{proc}.root' for year in l_year ]

    all_found = True 
    for path in l_path:
        if not os.path.isfile(path):
            all_found=False
            break

    return l_path, all_found
#-------------------
def get_input_paths(proc, dset):
    l_year = get_years(dset)

    l_path = [f'{data.dat_dir}/{proc}/{data.dat_vers}/{year}.root' for year in l_year ]
    for path in l_path:
        if not os.path.isfile(path):
            data.log.error(f'File {path} not found')
            raise FileNotFoundError

    return l_path
#-------------------
def get_df(year, trig, brem, is_data=None):
    if is_data not in [True, False]:
        data.log.error(f'Dataset type not specified')
        raise

    proc = 'data_ee' if is_data else 'ctrl_ee'
    utnr.make_dir_path('cached')
    l_cache_path, all_exist = get_cached_paths(year, trig, proc)
    if not all_exist:
        l_data_path = get_input_paths(proc, year)
        for data_path, cache_path in zip(l_data_path, l_cache_path):
            rdf = ROOT.RDataFrame('KEE', data_path)

            if data.fraction < 1.0:
                nentries = rdf.Count().GetValue()
                nkeep    = int(data.fraction * nentries)

                data.log.visible(f'Using {nkeep}/{nentries} entries')
                rdf = rdf.Range(nkeep)

            rdf = apply_selection(rdf, trig, year, proc)

            data.log.visible(f'Caching: {data_path}[KEE] -> {cache_path}[tree]')
            rdf.Snapshot('tree', cache_path)

    data.log.visible(f'Found cached files: {l_cache_path}[tree]')
    rdf = ROOT.RDataFrame('tree', l_cache_path)

    if brem == 2:
        rdf = rdf.Filter(f'nbrem>=     2')
    else:
        rdf = rdf.Filter(f'nbrem=={brem}')

    return rdf
#-------------------
def add_bdt(rdf, trig):
    rdf = rdf.Define('b_mass', data.b_mass)
    rdf = rdf.Define('j_mass', data.j_mass)
    rdf = rdf.Define('nbrem' , 'L1_BremMultiplicity + L2_BremMultiplicity')

    d_data        = rdf.AsNumpy(['b_mass', 'j_mass', 'nbrem', 'nSPDHits'])
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
def get_pdf(is_signal=None, split_by_nspd=None):
    if is_signal not in [True, False]:
        data.log.error('Signal flag not specified')
        raise

    if split_by_nspd not in [True, False]:
        data.log.error('split_by_nspd flag not specified')
        raise

    pdf = get_signal_pdf() if is_signal else get_full_pdf(split_by_nspd)

    return pdf
#-------------------
def float_pars(pdf):
    l_par    = list(pdf.get_params(floating=True)) + list(pdf.get_params(floating=False))
    data.log.info('Floating parameters:')
    for par in l_par:
        par.floating = True
        if par.name in data.d_sig_ini:
            val = data.d_sig_ini[par.name]
            par.set_value(val)

        data.log.info(f'{"":<4}{par.name:<20}{par.value():>20.3f}')
#-------------------
def reset_sig_pars(pdf, d_val):
    l_par    = list(pdf.get_params(floating=True)) + list(pdf.get_params(floating=False))
    data.log.info('Setting initial values:')
    for par in l_par:
        name = par.name
        if name not in d_val:
            continue

        val  = d_val[name]
        par.set_value(val)
        data.log.info(f'{name:<20}{"->":<10}{val:<10.3}')
#-------------------
def get_nspd_signal(l_pdf):
    fr1   = zfit.Parameter("fr1_nspd", 0.5,  0.0, 2./3.)
    fr2   = zfit.Parameter("fr2_nspd", 1./3., 0.0, 1.0, floating=False)

    pdf   = zfit.pdf.SumPDF(pdfs=l_pdf, fracs=[fr1, fr2])

    nsg   = zfit.Parameter("nsg", 10000, 0, 100000)
    epdf  = pdf.create_extended(nsg)

    return epdf
#-------------------
def get_cb_pdf(prefix):
    ap_r  = zfit.Parameter(f'ap_r{prefix}',  1.0,  -10.0, 10.0)
    pw_r  = zfit.Parameter(f'pw_r{prefix}',  1.0,    0.1, 10.0)
    sig_r = zfit.pdf.CrystalBall(obs=data.obs, mu=data.mu, sigma=data.sg, alpha=ap_r, n=pw_r)

    ap_l  = zfit.Parameter(f'ap_l{prefix}', -1.0,  -10.0, 10.0)
    pw_l  = zfit.Parameter(f'pw_l{prefix}',  1.0,    0.1,  10.)
    sig_l = zfit.pdf.CrystalBall(obs=data.obs, mu=data.mu, sigma=data.sg, alpha=ap_l, n=pw_l)

    if prefix == '':
        ncbr  = zfit.Parameter(f'ncbr{prefix}',  10,   0,  1000000)
        sig_r = sig_r.create_extended(ncbr)

        ncbl  = zfit.Parameter(f'ncbl{prefix}',  10,   0,  1000000)
        sig_l = sig_l.create_extended(ncbl) 

        sig   = zfit.pdf.SumPDF([sig_r, sig_l])
    else:
        fr    = zfit.Parameter(f'fr_cb{prefix}', 0.5,  0.0, 1)
        sig   = zfit.pdf.SumPDF([sig_r, sig_l], fracs=[fr])

    return sig
#-------------------
def get_signal_pdf(split_by_nspd = False, prefix=None):
    if data.sig_pdf is not None:
        return data.sig_pdf 

    if split_by_nspd:
        pdf_1 = get_cb_pdf(prefix='_1')
        pdf_2 = get_cb_pdf(prefix='_2')
        pdf_3 = get_cb_pdf(prefix='_3')

        sig   = get_nspd_signal([pdf_1, pdf_2, pdf_3])
    else:
        prefix = f'_{prefix}' if prefix is not None else ''
        sig    = get_cb_pdf(prefix)

    data.sig_pdf = sig

    return sig
#-------------------
def get_bkg_pdf():
    if data.bkg_pdf is not None:
        return data.bkg_pdf

    lam = zfit.Parameter('lam', -0.001, -0.1, -0.001)
    bkg = zfit.pdf.Exponential(lam=lam, obs=data.obs, name='Combinatorial')

    nbk = zfit.Parameter(f'nbk', 100, 0.0, 200000)
    bkg = bkg.create_extended(nbk)

    data.bkg_pdf = bkg

    return bkg
#-------------------
def get_full_pdf(split_by_nspd): 
    sig = get_signal_pdf(split_by_nspd)
    bkg = get_bkg_pdf()
    pdf = zfit.pdf.SumPDF([sig, bkg])

    data.log.debug(f'Signal    : {sig}')
    data.log.debug(f'Background: {bkg}')
    data.log.debug(f'Model     : {pdf}')

    return pdf
#-------------------
def fix_pdf(pdf, d_fix):
    if d_fix is None:
        return pdf

    l_par = list(pdf.get_params(floating=True))

    data.log.info('-----------------')
    data.log.info('Fixing parameters')
    data.log.info('-----------------')
    for par in l_par:
        if par.name not in d_fix:
            data.log.info(f'{par.name:<20}{"->":<10}{"floating":<20}')
            continue
        else:
            fix_val, _ = d_fix[par.name]

        par.assign(fix_val)
        par.floating=False

        data.log.info(f'{par.name:<20}{"->":<10}{fix_val:>20.3e}')

    return pdf
#-------------------
def fit(df, d_fix=None, identifier='unnamed'):
    jsn_path  = f'{data.plt_dir}/{identifier}.json'
    if os.path.isfile(jsn_path):
        data.log.info(f'Fit file found: {jsn_path}')
        d_par = utnr.load_json(jsn_path)
        return d_par

    is_signal = True if d_fix is None else False

    if   identifier.endswith('_nspd'):
        pdf = get_pdf(is_signal, split_by_nspd= True)
    elif identifier.endswith( '_nom'):
        pdf = get_pdf(is_signal, split_by_nspd=False)
    else:
        data.log.error(f'Invalid identifier: {identifier}')
        raise

    if is_signal:
        float_pars(pdf)
        if os.path.isfile(jsn_path):
            data.log.info(f'Loading cached simulation parameters: {jsn_path}')
            d_par = utnr.load_json(jsn_path)
            reset_sig_pars(pdf, d_par)

    pdf = fix_pdf(pdf, d_fix)
    dat = df.AsNumpy(['j_mass'])['j_mass']
    dat = dat[~numpy.isnan(dat)]

    obj=zfitter(pdf, dat)
    if is_signal:
        res=obj.fit(ntries=30, pval_threshold=0.04)
    else:
        res=obj.fit()

    if   res is None:
        plot_fit(dat, pdf, res, identifier)
        data.log.error(f'Fit failed')
        raise
    elif res.status != 0:
        data.log.error(f'Finished with status: {res.status}')
        print(res)
        raise

    res.hesse()
    res.freeze()

    plot_fit(dat, pdf, res, identifier)

    tex_path = f'{data.plt_dir}/{identifier}.tex'
    data.log.visible(f'Saving to: {tex_path}')
    result_to_latex(res, tex_path)

    pkl_path = f'{data.plt_dir}/{identifier}.pkl'
    utnr.dump_pickle(res, pkl_path)

    d_par = { name : [d_val['value'], d_val['hesse']['error']] for name, d_val in res.params.items() }

    utnr.dump_json(d_par, jsn_path)

    return d_par
#-------------------
def plot_fit(dat, pdf, res, identifier):
    obj   = zfp(data=dat, model=pdf, result=res)
    obj.plot(d_leg={}, plot_range=(2450, 3600))

    obj.axs[1].plot([2450, 3600], [0, 0], linestyle='--', color='black')

    plot_path = f'{data.plt_dir}/{identifier}.png'
    data.log.visible(f'Saving to: {plot_path}')
    plt.savefig(plot_path, bbox_inches='tight')
#-------------------
def get_fix_pars(d_par):
    d_fix = dict(d_par)
    for parname in d_par: 
        if parname.startswith('mu') or parname.startswith('sg'):
            del(d_fix[parname])

    if data.sys == 'nspd':
        for index in [1,2,3]:
            ryld = d_fix[f'ncbr_{index}'][0]
            lyld = d_fix[f'ncbl_{index}'][0]
            d_fix[f'fr_cb_{index}'] = [ryld / (ryld + lyld), 0]

            del(d_fix[f'ncbr_{index}'])
            del(d_fix[f'ncbl_{index}'])
    else:
        del(d_fix['ncbr'])
        del(d_fix['ncbl'])

    return d_fix
#-------------------
def add_nspd_col(df):
    arr_nspd = df.AsNumpy(['nSPDHits'])['nSPDHits']
    q1 = numpy.quantile(arr_nspd, 1./3)
    q2 = numpy.quantile(arr_nspd, 2./3)

    df = df.Define('nspd', f'float res=-1; if (nSPDHits<{q1}) res = 1; else if (nSPDHits < {q2}) res = 2; else res = 3; return res;')

    return df
#-------------------
def make_table(trig=None, year=None, brem=None):
    df_sim    = get_df(year, trig, brem, is_data=False)
    df_dat    = get_df(year, trig, brem, is_data= True)

    d_sim_par = {}
    if data.sys == 'nspd':
        df_sim    = add_nspd_col(df_sim)
        for i_nspd in [1,2,3]:
            df_sim_nspd = df_sim.Filter(f'nspd == {i_nspd}')
            d_tmp_1     = fit(df_sim_nspd, d_fix=None, identifier=f'sim_{trig}_{year}_{brem}_{i_nspd}_{data.sys}')
            d_tmp_2     = { f'{key}_{i_nspd}' : val for key, val in d_tmp_1.items() }
            d_sim_par.update(d_tmp_2)
    elif data.sys == 'nom':
        d_sim_par = fit(df_sim, d_fix=None, identifier=f'sim_{trig}_{year}_{brem}_{data.sys}')
    else:
        data.log.error(f'Invalid systematic: {data.sys}')
        raise

    if data.sim_only:
        return 

    d_fix_par = get_fix_pars(d_sim_par)
    _         = fit(df_dat, d_fix=d_fix_par, identifier=f'dat_{trig}_{year}_{brem}_{data.sys}')
#-------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to produce q2 smearing factors systematic tables')
    parser.add_argument('-v', '--vers' , type =str, help='Version, used for naming of output directory', required=True)
    parser.add_argument('-t', '--trig' , type =str, help='Trigger'                                     , choices=data.l_trig)
    parser.add_argument('-y', '--year' , type =str, help='Year'                                        , choices=data.l_year)
    parser.add_argument('-b', '--brem' , type =str, help='Brem category'                               , choices=data.l_brem)
    parser.add_argument('-s', '--sim'  ,            help='Do only simulation'                          , action='store_true')
    parser.add_argument('-x', '--sys'  , type =str, help='Systematic variabion'                        , choices=data.l_sys) 
    args = parser.parse_args()

    data.trig     = args.trig
    data.year     = args.year
    data.brem     = args.brem
    data.sim_only = args.sim
    data.sys      = args.sys
    data.plt_dir  = utnr.make_dir_path(f'plots/fits/{args.vers}')
#-------------------
if __name__ == '__main__':
    get_args()
    make_table(trig=data.trig, year=data.year, brem=data.brem)
#-------------------

