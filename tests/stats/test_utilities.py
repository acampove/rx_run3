'''
Module with unit tests for functions in dmu.stat.utilities
'''
import os
from importlib.resources import files

from dmu.logging.log_store import LogStore
from dmu.stats             import utilities as sut
from dmu.stats.zfit        import zfit
from dmu.stats.fitter      import Fitter
from dmu.stats.utilities   import print_pdf
from dmu.stats.utilities   import pdf_to_tex
from dmu.stats.utilities   import placeholder_fit
from dmu.stats.utilities   import is_pdf_usable

import pytest
import pandas as pnd
from zfit.core.basepdf import ZfitPDF

log = LogStore.add_logger('dmu:tests:stats:test_utilities')
#----------------------------------
class Data:
    '''
    data class
    '''
    fit_dir = '/tmp/tests/dmu/stats'
#----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:stats:utilities', 10)

    os.makedirs(Data.fit_dir, exist_ok=True)
    os.environ['CUDA_VISIBLE_DEVICES'] = '0'
#----------------------------------
def _get_pdf_simple() -> ZfitPDF:
    obs = zfit.Space('m',    limits=(-10, 10))
    mu  = zfit.Parameter("mu", 0.4,   -5,     5)
    sg  = zfit.Parameter("sg", 1.3,  0.5,     2)
    ne  = zfit.Parameter('ne',  10,    1, 10000)

    sg.floating = False

    pdf  = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)
    epdf = pdf.create_extended(ne)

    return epdf
#----------------------------------
def _get_pdf_composed_nonextended() -> ZfitPDF:
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
def _get_pdf(kind : str ='simple') -> ZfitPDF:
    if   kind == 'simple':
        return _get_pdf_simple()

    if kind == 'composed_nonextended':
        return _get_pdf_composed_nonextended()

    raise ValueError(f'Invalid PDF kind: {kind}')
#----------------------------------
def test_print_pdf():
    '''
    Tests for PDF printer
    '''
    pdf = _get_pdf(kind='composed_nonextended')

    d_const = {'mu1' : [0.0, 0.1], 'sg1' : [1.0, 0.1]}
    #-----------------
    print_pdf(pdf)

    print_pdf(pdf,
              blind   = ['sg.*', 'mu.*'])

    print_pdf(pdf,
              d_const = d_const,
              blind   = ['sg.*', 'mu.*'])
    #-----------------
    print_pdf(pdf,
              txt_path = f'{Data.fit_dir}/utilities/print_pdf/pdf.txt')

    print_pdf(pdf,
              blind    =['sg.*', 'mu.*'],
              txt_path = f'{Data.fit_dir}/utilities/print_pdf/pdf_blind.txt')

    print_pdf(pdf,
              d_const  = d_const,
              txt_path = f'{Data.fit_dir}/utilities/print_pdf/pdf_const.txt')
#----------------------------------
def test_pdf_to_tex():
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

    path = files('dmu_data').joinpath('tests/pdf_to_tex.txt')
    pdf_to_tex(path=path, d_par=d_par)
#----------------------------------
@pytest.mark.parametrize('make_plot', [True, False])
def test_placeholder_fit(make_plot : bool) -> None:
    '''
    Runs a placeholder fit needed to produce outputs useful
    to develop tools
    '''
    kind    = 'plotted' if make_plot else 'unplotted'
    fit_dir = f'{Data.fit_dir}/placeholder_{kind}'

    placeholder_fit(kind='s+b', fit_dir=fit_dir, plot_fit=make_plot)
#----------------------------------
def test_reuse_data() -> None:
    '''
    Tests running fit on cached data
    '''
    fit_dir = f'{Data.fit_dir}/placeholder_reuse_data'

    placeholder_fit(kind='s+b', fit_dir=fit_dir, plot_fit=False)

    df   = pnd.read_json(f'{fit_dir}/data.json')

    placeholder_fit(kind='s+b', fit_dir=fit_dir, plot_fit=True, df=df)
#----------------------------------
def test_is_pdf_usable():
    '''
    Tests for PDF printer
    '''
    pdf = _get_pdf(kind='composed_nonextended')

    is_pdf_usable(pdf)
#----------------------------------
def test_save_fit():
    '''
    Tests saving fit
    '''
    pdf = _get_pdf(kind='simple')
    dat = pdf.create_sampler(n=1000)

    obj = Fitter(pdf, dat)
    res = obj.fit()

    sut.save_fit(
            data   =dat,
            model  =pdf,
            res    =res,
            fit_dir=f'{Data.fit_dir}/save_fit/parametric')
#----------------------------------
