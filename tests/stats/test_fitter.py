'''
Module containing unit tests for Fitter class
'''

import os
import re

import ROOT
import zfit
import tqdm
import numpy
import matplotlib.pyplot           as plt
import pandas                      as pnd
import zutils.utils                as zfu

from zutils.plot   import plot     as zfp
from data_splitter import splitter as dsplit
from log_store     import log_store
from demu.stats.fitter        import Fitter

#-------------------------------------
def set_log():
    log_store.set_level('rx_scripts:Fitter', 10)
#-------------------------------------
class data:
    plt.rcParams.update({'figure.max_open_warning': 0})
    zfit.settings.changed_warnings.all = False

    nsample = 50000
    plt_dir = utnr.make_dir_path(f'tests/Fitter/plots')
    log     = utnr.getLogger(__name__)

    pdf     = None
    obs     = zfit.Space(f'm', limits=(-7.5, +7.5))
    arr     = numpy.random.normal(0, 1, size=100)
    df      = pnd.DataFrame({'x' : arr})
    zf      = zfit.Data.from_numpy(obs=obs, array=arr)
#-------------------------------------
def delete_all_pars():
    d_par = zfit.Parameter._existing_params
    l_key = list(d_par.keys())

    for key in l_key:
        del(d_par[key])
#-------------------------------------
def get_bounds():
    d_bound = {}
    d_bound['x'] = [0, 1 , 2, 3]
    d_bound['y'] = [0, 1 , 2, 3]
    #d_bound['z'] = [0, 1 , 2, 3]

    return d_bound
#-------------------------------------
def get_data(path):
    if os.path.isfile(path):
        rdf=ROOT.RDataFrame('tree', path)
        return rdf

    ROOT.gInterpreter.ProcessLine('TRandom3 r(1);')

    d_val = {}
    d_val['x'] = numpy.random.uniform(-1, 4, size=data.nsample)
    d_val['y'] = numpy.random.uniform(-1, 4, size=data.nsample)
    #d_val['z'] = numpy.random.uniform(-1, 4, size=data.nsample)

    rdf = ROOT.RDF.FromNumpy(d_val)
    rdf = rdf.Define('m', 'r.Gaus(x, 2 + y/4.)')
    rdf.Snapshot('tree', path)

    return rdf
#-------------------------------------
def get_pdf():
    if data.pdf is not None:
        return data.pdf

    mu  = zfit.Parameter(f'mu', 1.0, -5, 5)
    sg  = zfit.Parameter(f'sg', 1.3,  0, 5)

    pdf = zfit.pdf.Gauss(obs=data.obs, mu=mu, sigma=sg)

    nev = zfit.Parameter(f'nev', 100,  0, 10000000)
    pdf = pdf.create_extended(nev)

    data.pdf = pdf

    return pdf
#-------------------------------------
def run_splitter(dat_dir):
    if os.path.isfile(f'{dat_dir}/results.pkl'):
        return

    rdf     = get_data(f'{dat_dir}/data.root')

    d_bound = get_bounds()
    obj     = dsplit(rdf, d_bound, spectators=['m'])
    l_df    = obj.get_datasets()

    d_res = {}
    ndata = len(l_df)
    for i_df, df in tqdm.tqdm(enumerate(l_df), total=ndata, ascii=' -'):
        arr= df['m'].to_numpy()

        pdf= get_pdf()
        obj=Fitter(pdf, arr)
        res=obj.fit()
        res.hesse()
        res.freeze()

        mod = zfu.copy_model(pdf)
        d_res[i_df] = [arr, mod, res]

    utnr.dump_pickle(d_res, f'{dat_dir}/results.pkl')
#-------------------------------------
def strip_index(name):
    regex = '(.*)(_\d+$)'

    mtch  = re.match(regex, name)
    if not mtch:
        data.log.error(f'Cannnot strip index from {name}')
        raise

    return mtch.group(1)
#-------------------------------------
def get_row(res, l_col, i_row):
    l_val = []
    l_err = []
    for name in l_col:
        name = f'{name}_{i_row}'
        try:
            val = res.params[name]['value']
            err = res.params[name]['hesse']['error']
        except:
            data.log.error(f'Cannot extract {name} info from:')
            print(res.params)
            raise

        l_val.append(val)
        l_err.append(err)

    return l_val, l_err
#-------------------------------------
def plt_splitter(dat_dir):
    d_res = utnr.load_pickle(f'{dat_dir}/results.pkl')

    l_col= None
    df_v = None
    df_e = None
    for i_res, [dat, pdf, res] in d_res.items():
        if df_v is None:
            l_col= [ strip_index(var) for var in res.params ]
            df_v = pnd.DataFrame(columns=l_col)
            df_e = pnd.DataFrame(columns=l_col)

        obj=zfp(model=pdf, data=dat, result=res, suffix=f'{i_res}')
        obj.plot()
        plt.savefig(f'{data.plt_dir}/fit_{i_res:02}.png')
        plt.close('all')

        row_v, row_e = get_row(res, l_col, i_res)
        df_e = utnr.add_row_to_df(df_e, row_e)
        df_v = utnr.add_row_to_df(df_v, row_v)

    for col in l_col:
        df_v.plot(y=col, yerr=df_e)

        plt.savefig(f'{data.plt_dir}/{col}.png')
        plt.close('all')
#-------------------------------------
def test_splitter():
    pdf = get_pdf()
    dat_dir = utnr.make_dir_path('tests/Fitter/splitter/')

    run_splitter(dat_dir)
    delete_all_pars()
#-------------------------------------
def test_ntries():
    pdf = get_pdf()
    obj=Fitter(pdf, data.arr)
    res=obj.fit(ntries=10, pval_threshold=0.99)
    print(res)
    delete_all_pars()
#-------------------------------------
def do_test_simple(dat):
    pdf = get_pdf()
    obj=Fitter(pdf, dat)
    res=obj.fit()
    print(res)
    delete_all_pars()
#-------------------------------------
def test_simple_zf():
    do_test_simple(data.zf)
#-------------------------------------
def test_simple_arr():
    do_test_simple(data.arr)
#-------------------------------------
def test_simple_df():
    do_test_simple(data.df)
#-------------------------------------
def test_constrain():
    pdf = get_pdf()
    obj=Fitter(pdf, data.arr)

    res=obj.fit()
    res.hesse()
    print(res)

    res=obj.fit(d_const={'mu' : (0, 0.1), 'sg' : (1, 0.01), 'nev' : (100, 0)})
    res.hesse()
    print(res)
    delete_all_pars()
#-------------------------------------
def test_ranges():
    obs   = zfit.Space('x', limits=(0, 10))
    lb    = zfit.Parameter("lb", -1,  -2, 0)
    model = zfit.pdf.Exponential(obs=obs, lam=lb)
    nev   = zfit.Parameter('nev', 100, 0, 100000)
    epdf  = model.create_extended(nev)
    data  = numpy.random.exponential(5, size=10000)
    data  = data[(data < 10)]
    data  = data[(data < 2) | ((data > 4) & (data < 6)) |  ((data > 8) & (data < 10)) ]

    obj   = Fitter(epdf, data)
    rng   = [(0,2), (4, 6), (8, 10)]
    res   = obj.fit(ranges=rng)

    obj   = zfp(data=data, model=epdf, result=res)
    obj.plot(nbins=50, ranges=rng)
    obj.axs[1].set_ylim(-5, +5)
    obj.axs[1].axhline(y=0, color='r')

    os.makedirs('tests/test_ranges', exist_ok=True)
    plot_path = f'tests/test_ranges/fit.png'

    plt.savefig(plot_path)
    delete_all_pars()
#-------------------------------------
def test_wgt():
    dat_dir = utnr.make_dir_path('tests/Fitter/splitter/')
    rdf = get_data(f'{dat_dir}/data.root')
    arr = rdf.AsNumpy(['m'])['m']
    wgt = numpy.random.binomial(1, 0.5, size=arr.size)

    pdf = get_pdf()
    dat = zfit.Data.from_numpy(array=arr, weights=wgt, obs=pdf.space)

    obj=Fitter(pdf, dat)
    res=obj.fit()
    res.hesse(method='minuit_hesse')

    print(res)

    delete_all_pars()
