'''
Script needed to calculate smearing factors for q2 distribution
'''

import os
import re
import glob
import argparse

import hist
import numpy
import mplhep
import matplotlib.pyplot   as plt

import ROOT # ROOT import has to be before zfit ones to avoid crash due to ROOT -> tensorflow issue
from ROOT import RDataFrame

import zfit
from zfit.core.data         import Data       as zdata
from zfit.core.basepdf      import BasePDF    as zpdf
from zfit.core.interfaces   import ZfitSpace  as zobs
from zfit.core.parameter    import Parameter  as zpar
from zfit.result            import FitResult  as zres

import dmu.generic.utilities as gut
from dmu.stats              import utilities   as sut
from dmu.stats.fitter       import Fitter      as zfitter
from dmu.stats.zfit_plotter import ZFitPlotter as zfp
from dmu.logging.log_store  import LogStore
from rx_q2.data_set         import data_set

log=LogStore.add_logger('rx_q2:get_q2_tables')
#-------------------
class Data:
    '''
    Data class
    '''
    zfit.settings.changed_warnings.hesse_name = False

    cal_dir  = os.environ['CALDIR']
    dat_dir  = os.environ['DATDIR']
    cas_dir  = os.environ['CASDIR']
    mva_dir  = os.environ['MVADIR']
    qsq_dir  = os.environ['QSQSYS']

    l_run3_year = ['2024']

    l_year   = ['2011', '2012', '2015', '2016', '2017', '2018', 'r1', 'r2p1', '2024']
    l_trig   = ['ETOS', 'GTIS']
    l_brem   = ['0', '1', '2']
    l_sys    = ['nom', 'nspd']
    l_sam    = ['simulation', 'data', 'both']
    l_cali   = ['000', 'nom']

    bdt_dir  = f'{mva_dir}/electron/bdt_v10.14.a0v2ss'
    b_mass   = 'B_const_mass_M[0]'
    j_mass   = 'Jpsi_M'
    nbins    = 60

    trig     = None
    year     = None
    brem     = None
    plt_dir  = None
    nentries = None
    skip_fit = None
    nevs_data= None
    cal_sys  = None
    obs      : zobs

    sig_pdf_splt = None
    sig_pdf_merg = None
    bkg_pdf      = None
    d_sim_par    : dict[str,tuple[float,float]]

    mu       = zfit.Parameter('mu', 3060,  3040, 3100)
    sg       = zfit.Parameter('sg',   20,    10,  100)

    dmu      = zfit.Parameter('dmu', 0, -50.0, 50.0)
    rsg      = zfit.Parameter('rsg', 1, 0.7,  1.4)

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
def _float_pars(pdf : zpdf) -> None:
    l_par    = list(pdf.get_params(floating=True)) + list(pdf.get_params(floating=False))
    log.info('Floating parameters:')
    for par in l_par:
        par.floating = True
        if par.name in Data.d_sig_ini:
            val = Data.d_sig_ini[par.name]
            par.set_value(val)

        log.info(f'{"":<4}{par.name:<20}{par.value():>20.3f}')
#-------------------
def _reset_sig_pars(pdf : zpdf, d_val : dict[str,tuple[float,float]]) -> None:
    l_par    = list(pdf.get_params(floating=True)) + list(pdf.get_params(floating=False))
    log.info('Setting initial values:')
    for par in l_par:
        name = par.name
        if name not in d_val:
            continue

        val  = d_val[name]
        par.set_value(val)
        log.info(f'{name:<20}{"->":<10}{val:<10.3}')
#-------------------
def _get_nspd_signal(l_pdf : list[zpdf]) -> zpdf:
    nsg_1 = zfit.Parameter('nsg_1', 1000, 0, Data.nevs_data)
    nsg_2 = zfit.Parameter('nsg_2', 1000, 0, Data.nevs_data)
    nsg_3 = zfit.Parameter('nsg_3', 1000, 0, Data.nevs_data)

    l_nsg = [nsg_1, nsg_2, nsg_3]
    l_epdf= [ pdf.create_extended(nsg) for pdf, nsg in zip(l_pdf, l_nsg) ]
    epdf  = zfit.pdf.SumPDF(pdfs=l_epdf)

    return epdf
#-------------------
def _get_cb_pdf() -> zpdf:
    ap_r  = zfit.Parameter('ap_r',  1.0,  -3.0,   3.0)
    pw_r  = zfit.Parameter('pw_r',  1.0,   0.1,  20.0)
    sig_r = zfit.pdf.CrystalBall(obs=Data.obs, mu=Data.mu, sigma=Data.sg, alpha=ap_r, n=pw_r)

    ap_l  = zfit.Parameter('ap_l', -1.0,   -3.0,  3.0)
    pw_l  = zfit.Parameter('pw_l',  1.0,    0.1, 20.0)
    sig_l = zfit.pdf.CrystalBall(obs=Data.obs, mu=Data.mu, sigma=Data.sg, alpha=ap_l, n=pw_l)

    ncbr  = zfit.Parameter('ncbr',  10,   0,  1000000)
    sig_r = sig_r.create_extended(ncbr, name='CB1')

    ncbl  = zfit.Parameter('ncbl',  10,   0,  1000000)
    sig_l = sig_l.create_extended(ncbl, name='CB2')

    sig   = zfit.pdf.SumPDF([sig_r, sig_l], name='Signal')

    return sig
#-------------------
def _get_nspd_data_pars(preffix : str ='') -> tuple[zpar,zpar]:
    sim_mu = zfit.param.ConstantParameter(f'sim_mu{preffix}', Data.d_sim_par[f'mu{preffix}'][0])
    sim_sg = zfit.param.ConstantParameter(f'sim_sg{preffix}', Data.d_sim_par[f'sg{preffix}'][0])

    dat_mu = zfit.ComposedParameter(f'dat_mu{preffix}',
                                    func=lambda d_par : d_par['dmu'] + d_par[f'sim_mu{preffix}'],
                                    params={'dmu' : Data.dmu, f'sim_mu{preffix}' : sim_mu} )
    dat_sg = zfit.ComposedParameter(f'dat_sg{preffix}',
                                    func=lambda d_par : d_par['rsg'] * d_par[f'sim_sg{preffix}'],
                                    params={'rsg' : Data.rsg, f'sim_sg{preffix}' : sim_sg} )

    return dat_mu, dat_sg
#-------------------
def _get_cb_nspd_pdf(prefix : str = '') -> zpdf:
    mu,sg = _get_nspd_data_pars(prefix)

    ap_r  = zfit.Parameter(f'ap_r{prefix}',  1.0,  -10.0, 10.0)
    pw_r  = zfit.Parameter(f'pw_r{prefix}',  1.0,    0.1, 10.0)
    sig_r = zfit.pdf.CrystalBall(obs=Data.obs, mu=mu, sigma=sg, alpha=ap_r, n=pw_r)

    ap_l  = zfit.Parameter(f'ap_l{prefix}', -1.0,  -10.0, 10.0)
    pw_l  = zfit.Parameter(f'pw_l{prefix}',  1.0,    0.1,  10.)
    sig_l = zfit.pdf.CrystalBall(obs=Data.obs, mu=mu, sigma=sg, alpha=ap_l, n=pw_l)

    fr    = zfit.Parameter(f'fr_cb{prefix}', 0.5,  0.0, 1)
    sig   = zfit.pdf.SumPDF([sig_r, sig_l], fracs=[fr])

    return sig
#-------------------
def _get_signal_pdf(split_by_nspd : bool = False) -> zpdf:
    if Data.sig_pdf_splt is not None and     split_by_nspd:
        return Data.sig_pdf_splt

    if Data.sig_pdf_merg is not None and not split_by_nspd:
        return Data.sig_pdf_merg

    if split_by_nspd:
        l_pdf = [ _get_cb_nspd_pdf(prefix=f'_{i_nspd}') for i_nspd in [1, 2, 3] ]
        Data.sig_pdf_splt = _get_nspd_signal(l_pdf)
    else:
        Data.sig_pdf_merg = _get_cb_pdf()

    return Data.sig_pdf_splt if split_by_nspd else Data.sig_pdf_merg
#-------------------
def _get_bkg_pdf() -> zpdf:
    if Data.bkg_pdf is not None:
        return Data.bkg_pdf

    lam = zfit.Parameter('lam', -0.001, -0.1, -0.0001)
    bkg = zfit.pdf.Exponential(lam=lam, obs=Data.obs, name='')

    nbk = zfit.Parameter('nbk', 100, 0.0, 200000)
    bkg = bkg.create_extended(nbk, name='Combinatorial')

    Data.bkg_pdf = bkg

    return bkg
#-------------------
def _get_full_pdf(split_by_nspd : bool):
    sig = _get_signal_pdf(split_by_nspd)
    bkg = _get_bkg_pdf()
    pdf = zfit.pdf.SumPDF([sig, bkg], name='Model')

    log.debug(f'Signal    : {sig}')
    log.debug(f'Background: {bkg}')
    log.debug(f'Model     : {pdf}')

    return pdf
#-------------------
def _fix_pdf(
        pdf   : zpdf,
        d_fix : dict[str,tuple[float,float]]) -> zpdf:

    if d_fix is None:
        return pdf

    l_par = list(pdf.get_params(floating=True))

    log.info('-----------------')
    log.info('Fixing parameters')
    log.info('-----------------')
    for par in l_par:
        if re.match(r'mu_[1,2,3]', par.name) or re.match(r'sg_[1,2,3]', par.name):
            continue

        if par.name not in d_fix:
            log.info(f'{par.name:<20}{"->":<10}{"floating":>20}')
            continue

        fix_val, _ = d_fix[par.name]
        par.set_value(fix_val)
        par.floating=False

        log.info(f'{par.name:<20}{"->":<10}{fix_val:>20.3e}')

    return pdf
#-------------------
def _get_pdf(is_signal : bool, split_by_nspd : bool) -> zpdf:
    pdf = _get_signal_pdf() if is_signal else _get_full_pdf(split_by_nspd)

    return pdf
#-------------------
def _get_pars(res : zres, identifier : str) -> dict[str,list[str]]:
    try:
        d_par = { name : [ d_val['value'], d_val['hesse']['error'] ] for name, d_val in res.params.items() }
    except Exception as exc:
        log.info(res)
        log.info(res.params)
        raise ValueError(f'Cannot extract {identifier} parameters from:') from exc

    return d_par
#-------------------
def _fit(
        rdf        : RDataFrame,
        d_fix      : dict[str,tuple[float,float]] = None,
        identifier : str                          ='unnamed') -> dict[str,tuple[float,float]]:
    fit_dir  = f'{Data.plt_dir}/{identifier}'
    jsn_path = f'{fit_dir}/parameters.json'
    if os.path.isfile(jsn_path):
        log.info(f'Fit file found: {jsn_path}')
        d_par = gut.load_json(jsn_path)
        return d_par

    is_signal = d_fix is None

    if   identifier.endswith('_nspd'):
        log.info(f'Splitting by nSPD: {identifier}')
        pdf = _get_pdf(is_signal, split_by_nspd= True)
    elif identifier.endswith( '_nom'):
        log.info(f'Not splitting by nSPD: {identifier}')
        pdf = _get_pdf(is_signal, split_by_nspd=False)
    else:
        raise ValueError(f'Invalid identifier: {identifier}')

    if is_signal:
        _float_pars(pdf)
        if os.path.isfile(jsn_path):
            log.info(f'Loading cached simulation parameters: {jsn_path}')
            d_par = gut.load_json(jsn_path)
            _reset_sig_pars(pdf, d_par)

    dat = _get_data(rdf, pdf, is_signal, identifier)
    pdf = _fix_pdf(pdf, d_fix)
    obj = zfitter(pdf, dat)

    if Data.skip_fit:
        log.warning('Skipping fit')
        return None

    if is_signal:
        res=obj.fit(cfg={'strategy' : {'retry' : {'ntries' : 10}}})
    else:
        res=obj.fit()

    if res is None:
        _plot_fit(dat, pdf, res, identifier, add_pars=None)
        _plot_fit(dat, pdf, res, identifier, add_pars='all')

        log.info(res)
        raise ValueError('Fit failed')

    if res.status != 0:
        _plot_fit(dat, pdf, res, identifier, add_pars=None)
        _plot_fit(dat, pdf, res, identifier, add_pars='all')

        log.info(res)
        raise ValueError(f'Finished with status/validity: {res.status}/{res.valid}')

    log.info('Found parameters:')
    log.info(res)
    _plot_fit(dat, pdf, res, identifier, add_pars='all')

    d_par = _get_pars(res, identifier)

    sut.save_fit(data=dat, model=pdf, res=res, fit_dit=fit_dir)

    return d_par
#-------------------
def _get_data(
        rdf        : RDataFrame,
        pdf        : zpdf,
        is_signal  : bool,
        identifier : str) -> zdata:
    arr_val = rdf.AsNumpy(['Jpsi_M'])['Jpsi_M']
    arr_wgt = rdf.AsNumpy(['weight'])['weight']

    obs     = pdf.space
    dat     = zfit.Data.from_numpy(obs=obs, array=arr_val, weights=arr_wgt)

    _plot_data(arr_val, arr_wgt, obs, is_signal, identifier)

    return dat
#-------------------
def _get_obs_range() -> tuple[int,int]:
    if Data.brem == '0':
        return [2200, 3300]

    return [2200, 3800]
#-------------------
def _plot_data(
        arr_mas    : numpy.ndarray,
        arr_wgt    : numpy.ndarray,
        obs        : zobs,
        is_signal  : bool,
        identifier : str) -> None:

    [[lower]], [[upper]] = obs.limits
    _, ax     = plt.subplots(figsize=(15, 10))
    data_hist = hist.Hist.new.Regular(Data.nbins, lower, upper, name='', underflow=False, overflow=False)
    data_hist = data_hist.Weight()
    data_hist.fill(arr_mas, weight=arr_wgt)

    mplhep.histplot(
        data_hist,
        yerr    = True,
        color   = 'black',
        histtype= 'errorbar',
        ax      = ax
    )

    title=f'Entries={arr_wgt.size:.0f}; Sum={numpy.sum(arr_wgt):.0f}; {identifier}'

    if is_signal:
        plt_path = f'{Data.plt_dir}/ctrl_{identifier}.png'
    else:
        plt_path = f'{Data.plt_dir}/data_{identifier}.png'

    log.info(f'Saving to: {plt_path}')
    plt.title(title)
    plt.savefig(plt_path)
    plt.close('all')
#-------------------
def _plot_fit(
        dat        : zdata,
        pdf        : zpdf,
        res        : zres,
        identifier : str,
        add_pars   : str) -> None:

    obs_range = _get_obs_range()
    obj=zfp(data=dat, model=pdf, result=res)
    obj.plot(nbins=Data.nbins, d_leg={}, plot_range=obs_range, ext_text=f'#events={dat.nevents.numpy()}', add_pars=add_pars)
    obj.axs[1].plot(obs_range, [0, 0], linestyle='--', color='black')

    if add_pars is not None:
        plot_path = f'{Data.plt_dir}/{identifier}_pars.png'
    else:
        plot_path = f'{Data.plt_dir}/{identifier}.png'

    log.info(f'Saving to: {plot_path}')
    plt.savefig(plot_path, bbox_inches='tight')
    plt.close()
#-------------------
def _get_fix_pars(d_par : dict[str,tuple[float,float]]) -> dict[str,tuple[float,float]]:
    d_fix = dict(d_par)
    for parname in d_par:
        if parname.startswith('mu') or parname.startswith('sg'):
            del d_fix[parname]

    if Data.sys == 'nspd':
        for index in [1,2,3]:
            ryld = d_fix[f'ncbr_{index}'][0]
            lyld = d_fix[f'ncbl_{index}'][0]
            d_fix[f'fr_cb_{index}'] = [ryld / (ryld + lyld), 0]

            del d_fix[f'ncbr_{index}']
            del d_fix[f'ncbl_{index}']
    else:
        del d_fix['ncbr']
        del d_fix['ncbl']

    return d_fix
#-------------------
def _add_nspd_col(rdf : RDataFrame) -> RDataFrame:
    if Data.year in Data.l_run3_year:
        log.info(f'Skipping adding nSPDHits for {Data.year}')
        return rdf

    arr_nspd = rdf.AsNumpy(['nSPDHits'])['nSPDHits']
    q1       = numpy.quantile(arr_nspd, 1./3)
    q2       = numpy.quantile(arr_nspd, 2./3)
    rdf      = rdf.Define('nspd', f'float res=-1; if (nSPDHits<{q1}) res = 1; else if (nSPDHits < {q2}) res = 2; else res = 3; return res;')

    return rdf
#-------------------
def _get_sim_pars_cache() -> dict[str,float]:
    json_wc = f'{Data.plt_dir}/sim_{Data.trig}_{Data.year}_{Data.brem}*.json'
    l_json_path = glob.glob(json_wc)
    if   len(l_json_path) == 0:
        raise ValueError(f'No file found in: {json_wc}')

    if len(l_json_path) == 1:
        json_path = l_json_path[0]
        d_par     = gut.load_json(json_path)
    else:
        d_par = {}
        for i_nspd, json_path in enumerate(l_json_path):
            log.info(f'Loading parameters from: {json_path}')
            d_par_x = gut.load_json(json_path)
            d_par_r = {f'{key}_{i_nspd + 1}' : val for key, val in d_par_x.items()}
            d_par.update(d_par_r)

    return d_par
#-------------------
def _get_sim_pars_fits(rdf : RDataFrame, identifier : str):
    d_par = {}
    if   Data.sys == 'nspd':
        rdf = _add_nspd_col(rdf)
        for i_nspd in [1,2,3]:
            rdf_sim_nspd = rdf.Filter(f'nspd == {i_nspd}')
            d_tmp_1      = _fit(rdf_sim_nspd, d_fix=None, identifier=f'sim_{i_nspd}_{identifier}')
            d_tmp_2      = { f'{key}_{i_nspd}' : val for key, val in d_tmp_1.items() }
            d_par.update(d_tmp_2)
    elif Data.sys == 'nom':
        d_par = _fit(rdf, d_fix=None, identifier=f'sim_{identifier}')
    else:
        raise ValueError(f'Invalid systematic: {Data.sys}')

    return d_par
#-------------------
def _make_table():
    identifier= f'{Data.trig}_{Data.year}_{Data.brem}_{Data.sys}'

    odf_sim   = data_set(is_mc= True, trigger=Data.trig, dset=Data.year)
    odf_sim.plt_dir = f'{Data.plt_dir}/cal_wgt_sim_{identifier}'
    rdf_sim    = odf_sim.get_rdf()
    rdf_sim    = rdf_sim.Filter(f'nbrem == {Data.brem}')

    odf_dat   = data_set(is_mc=False, trigger=Data.trig, dset=Data.year)
    odf_dat.plt_dir = f'{Data.plt_dir}/dat_plt_{identifier}'
    rdf_dat    = odf_dat.get_rdf()
    rdf_dat    = rdf_dat.Filter(f'nbrem == {Data.brem}')

    if Data.sam == 'data':
        d_sim_par = _get_sim_pars_cache()
    else:
        d_sim_par = _get_sim_pars_fits(rdf_sim, identifier)

    if Data.sam == 'simulation':
        log.info('Done with simulation and returning')
        return

    Data.d_sim_par = d_sim_par
    Data.nevs_data = rdf_dat.Count().GetValue()

    d_fix_par = _get_fix_pars(d_sim_par)
    _         = fit(rdf_dat, d_fix=d_fix_par, identifier=f'dat_{Data.trig}_{Data.year}_{Data.brem}_{Data.sys}')
#-------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Used to produce q2 smearing factors systematic tables')
    parser.add_argument('-v', '--vers' , type =str, help='Version, used for naming of output directory', required=True)
    parser.add_argument('-t', '--trig' , type =str, help='Trigger'                                     , required=True, choices=Data.l_trig)
    parser.add_argument('-y', '--year' , type =str, help='Year'                                        , required=True, choices=Data.l_year)
    parser.add_argument('-b', '--brem' , type =str, help='Brem category'                               , required=True, choices=Data.l_brem)
    parser.add_argument('-c', '--cali' , type =str, help='Calibration weight systematics'              , default='nom', choices=Data.l_cali)
    parser.add_argument('-x', '--sys'  , type =str, help='Systematic variabion'                        , choices=Data.l_sys)
    parser.add_argument('-s', '--sam'  , type =str, help='Sample'                                      , choices=Data.l_sam, default='both')
    parser.add_argument('-e', '--nent' , type =int, help='Number of entries to run over, for tests'    , default=-1)
    parser.add_argument('--skip_fit'   , help='Will not fit, just plot the model'                      , action='store_true')
    args = parser.parse_args()

    Data.trig     = args.trig
    Data.year     = args.year
    Data.brem     = args.brem
    Data.sys      = args.sys
    Data.sam      = args.sam
    Data.cal_sys  = args.cali
    Data.nentries = args.nent
    Data.skip_fit = args.skip_fit

    syst          = {'nom' : 'nom', 'nspd' : 'lsh'}[Data.sys]
    Data.plt_dir  = f'{Data.qsq_dir}/get_q2_tables/fits/{args.vers}.{syst}'
    os.makedirs(Data.plt_dir, exist_ok=True)
    Data.obs      = zfit.Space('Jpsi_M', limits=_get_obs_range())
#-------------------
def main():
    '''
    Start here
    '''
    plt.style.use(mplhep.style.LHCb2)

    _get_args()
    _make_table()
#-------------------
if __name__ == '__main__':
    main()
