'''
Module with unit tests for functions in dmu.stat.utilities
'''
import os
import re
import math
from importlib.resources import files
from pathlib             import Path

import numpy
from omegaconf              import OmegaConf
from dmu.logging.log_store  import LogStore
from dmu.stats              import utilities as sut
from dmu.stats.zfit         import zfit
from dmu.stats.fitter       import Fitter
from dmu.stats.utilities    import print_pdf
from dmu.stats.utilities    import pdf_to_tex
from dmu.stats.utilities    import placeholder_fit
from dmu.stats.utilities    import is_pdf_usable

import pytest
import pandas as pnd
from zfit.data        import Data     as zdata
from zfit.pdf         import BasePDF  as zpdf

log = LogStore.add_logger('dmu:tests:stats:test_utilities')
#----------------------------------
class Data:
    '''
    Class needed to hold shared variables
    '''
    user    = os.environ['USER']
#----------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:stats:utilities', 10)
#----------------------------------
def _get_pdf_simple(is_extended : bool = True) -> zpdf:
    obs = zfit.Space('m',    limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.4,   -5,     5)
    sg  = zfit.Parameter("sg", 1.3,  0.5,     2)
    ne  = zfit.Parameter('ne',  10,    1, 10000)

    sg.floating = False

    pdf  = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)
    if not is_extended:
        return pdf

    epdf = pdf.create_extended(ne)

    return epdf
#----------------------------------
def _get_pdf_composed_nonextended() -> zpdf:
    obs = zfit.Space('m', limits=(-10, 10))
    mu1 = zfit.Parameter("mu1", 0.4, -5, 5)
    sg1 = zfit.Parameter("sg1", 1.3,  0, 5)
    mu2 = zfit.Parameter("mu2", 0.4, -5, 5)
    sg2 = zfit.Parameter("sg2", 1.3,  0, 5)

    pd1 = zfit.pdf.Gauss(obs=obs, mu=mu1, sigma=sg1, name='gauss1')
    fr1 = zfit.Parameter('fr1', 0.5, 0, 1)

    pd2 = zfit.pdf.Gauss(obs=obs, mu=mu2, sigma=sg2, name='gauss2')
    fr2 = zfit.Parameter('fr2', 0.5, 0, 1)

    pdf = zfit.pdf.SumPDF([pd1, pd2], fracs=[fr1, fr2])

    return pdf
#----------------------------------
def _get_zdata(weighted : bool) -> tuple[zdata, float]:
    '''
    Returns zfit data, weighted or not and yield
    '''
    arr_val = numpy.random.uniform(0, 1 , size=100)
    if weighted:
        arr_wgt = numpy.random.normal(1, 0.1, size=100)
        target  = arr_wgt.sum()
    else:
        arr_wgt = None
        target  = 100

    obs  = zfit.Space('x', limits=(0, 1))
    data = zfit.data.from_numpy(obs=obs, array=arr_val, weights=arr_wgt)

    return data, target
#----------------------------------
def _get_pdf(kind : str ='simple') -> zpdf:
    if   kind == 'simple':
        return _get_pdf_simple()

    if kind == 'composed_nonextended':
        return _get_pdf_composed_nonextended()

    raise ValueError(f'Invalid PDF kind: {kind}')
#----------------------------------
def test_print_pdf(tmp_path : Path):
    '''
    Tests for PDF printer
    '''
    pdf = _get_pdf(kind='composed_nonextended')

    d_const = {'mu1' : (0.0, 0.1), 'sg1' : (1.0, 0.1)}
    #-----------------
    print_pdf(pdf)

    print_pdf(pdf,
              blind   = ['sg.*', 'mu.*'])

    print_pdf(pdf,
              d_const = d_const,
              blind   = ['sg.*', 'mu.*'])
    #-----------------
    print_pdf(pdf,
              txt_path = f'{tmp_path}/utilities/print_pdf/pdf.txt')

    print_pdf(pdf,
              blind    =['sg.*', 'mu.*'],
              txt_path = f'{tmp_path}/utilities/print_pdf/pdf_blind.txt')

    print_pdf(pdf,
              d_const  = d_const,
              txt_path = f'{tmp_path}/utilities/print_pdf/pdf_const.txt')
# ----------------------
def test_blind_manager():
    '''
    Tests blinded_variables context manager
    '''
    pdf   = _get_pdf(kind='composed_nonextended')
    regex = r'mu.*'

    with sut.blinded_variables(regex_list=[regex]):
        l_msg = print_pdf(pdf=pdf)

    assert isinstance(l_msg, list)

    for line in l_msg:
        assert not re.match(regex, line)
#----------------------------------
def test_pdf_to_tex(tmp_path : Path):
    '''
    Tests converting text file with PDF description
    into latex table
    '''
    d_par = {
            'ar_dscb_Signal_002_1_reso_flt' : r'$\alpha_{DSCB}^{1}$',
            'c_exp_cmb_1'                   : 'b',
            'frac_brem_001'                 : 'c',
            'mu_Signal_001_scale_flt'       : 'd',
            'mu_Signal_002_scale_flt'       : 'e',
            }

    in_path = files('dmu_data').joinpath('tests/pdf_to_tex.txt')
    pdf_to_tex(in_path=in_path, out_dir = tmp_path, d_par=d_par)
#----------------------------------
@pytest.mark.parametrize('make_plot', [True, False])
def test_placeholder_fit(make_plot : bool, tmp_path : Path) -> None:
    '''
    Runs a placeholder fit needed to produce outputs useful
    to develop tools
    '''
    placeholder_fit(
        kind     = 's+b',
        fit_dir  = tmp_path,
        plot_fit = make_plot)
# ----------------------
@pytest.mark.parametrize('suffix', [None ,   'suff'])
@pytest.mark.parametrize('kind'  , ['s+b', 'signal'])
def test_get_placeholder_model(
    kind   : str,
    suffix : str|None) -> None:
    '''
    Test of get_model
    '''
    pdf = sut.get_model(kind=kind, suffix=suffix)
    sut.is_pdf_usable(pdf=pdf)
# ----------------------
@pytest.mark.parametrize('kind'  , ['s+b', 'signal'])
def test_get_placeholder_nll(kind : str) -> None:
    '''
    Test of get_nll
    '''
    _ = sut.get_nll(kind=kind)
#----------------------------------
def test_reuse_data(tmp_path : Path) -> None:
    '''
    Tests running fit on cached data
    '''
    placeholder_fit(kind='s+b', fit_dir=tmp_path, plot_fit=False)

    df   = pnd.read_json(f'{tmp_path}/data.json')

    placeholder_fit(kind='s+b', fit_dir=tmp_path, plot_fit=True, df=df)
#----------------------------------
def test_is_pdf_usable():
    '''
    Tests for PDF printer
    '''
    pdf = _get_pdf(kind='composed_nonextended')

    is_pdf_usable(pdf=pdf)
#----------------------------------
@pytest.mark.parametrize('is_extended', [True, False])
def test_save_fit_simple(tmp_path : Path, is_extended : bool):
    '''
    Simplest case
    '''
    pdf = _get_pdf_simple(is_extended=is_extended)
    dat = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, dat)
    res = obj.fit()

    cfg = {
        'nbins'      : 50, 
        'stacked'    : True, 
        'plot_range' : [-5, +6],
        'yrange'     : {
            'log'    : [1.0, 1e3],
            'linear' : [0.0, 1e2]}
    }

    measurement = sut.save_fit(
        data   =dat,
        model  =pdf,
        res    =res,
        plt_cfg=cfg,
        fit_dir=tmp_path / 'save_fit/simple')

    if not is_extended:
        assert 'nentries' in measurement
    else:
        assert 'nentries' not in measurement
#----------------------------------
def test_save_fit_param(tmp_path : Path):
    '''
    Tests saving fit with parameters
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, dat)
    res = obj.fit()

    sut.save_fit(
        data   =dat,
        model  =pdf,
        res    =res,
        plt_cfg={'nbins' : 50, 'stacked' : True},
        fit_dir=tmp_path)
#----------------------------------
def test_save_fit_nomodel(tmp_path : Path):
    '''
    Tests saving fit without model 
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    sut.save_fit(
        data   =dat,
        model  =None,
        res    =None,
        plt_cfg={'nbins' : 50, 'stacked' : True},
        fit_dir=tmp_path)
#----------------------------------
def test_save_fit_param_refreeze(tmp_path : Path):
    '''
    Tests saving fit with parameters
    when the result object has already been frozen
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, dat)
    res = obj.fit()
    res.freeze()

    sut.save_fit(
        data   =dat,
        model  =pdf,
        res    =res,
        plt_cfg={'nbins' : 50, 'stacked' : True},
        fit_dir=tmp_path)
#----------------------------------
def test_name_from_obs():
    '''
    Tests retrieval of name from observable
    '''
    obs  = zfit.Space('xyz', limits=(0, 10))
    name = sut.name_from_obs(obs=obs)

    assert name == 'xyz'
#----------------------------------
def test_zres_to_cres(tmp_path : Path):
    '''
    Tests conversion of zfit result object to
    DictConfig
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, dat)
    res = obj.fit()
    res.freeze()

    cres = sut.zres_to_cres(res=res)

    OmegaConf.save(config=cres, f= tmp_path / 'results.yaml')
#----------------------------------
def test_zres_to_cres_fallback():
    '''
    Tests conversion of zfit result object to
    DictConfig when errors are missing
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    with Fitter.errors_disabled(value=True):
        obj = Fitter(pdf, dat)
        res = obj.fit()
        res.freeze()

    with pytest.raises(KeyError):
        sut.zres_to_cres(res=res)

    cres = sut.zres_to_cres(res=res, fall_back_error=-1)
    for data in cres.values():
        assert math.isclose(data.error, -1.0, rel_tol=1e-5)
#----------------------------------
def test_range_from_obs():
    '''
    Tests retrieval of range from observable
    '''
    obs        = zfit.Space('xyz', limits=(0, 10))
    minx, maxx = sut.range_from_obs(obs=obs)

    assert minx ==  0
    assert maxx == 10
#----------------------------------
@pytest.mark.parametrize('weighted', [True, False])
def test_yield_from_zdata(weighted : bool):
    '''
    Tests function that does retrieval of yield from zfit data
    '''
    data, target = _get_zdata(weighted=weighted)

    val = sut.yield_from_zdata(data=data)

    assert abs(val - target) < 1e-5
#----------------------------------
def test_val_from_zres() -> None:
    '''
    Tests `val_from_zres`
    '''
    expected = 5200.384730974302

    res = placeholder_fit(kind='s+b', fit_dir=None)
    log.info(res)

    val = sut.val_from_zres(res=res, name='mu')
    assert math.isclose(val, expected, rel_tol=1e-5)

    res.freeze()
    val = sut.val_from_zres(res=res, name='mu')
    assert math.isclose(val, expected, rel_tol=1e-5)

    with pytest.raises(ValueError):
        sut.val_from_zres(res=res, name='fake')
