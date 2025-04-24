'''
Module with unit tests for functions in dmu.stat.utilities
'''
from importlib.resources import files

import pytest
import zfit
from zfit.core.basepdf import ZfitPDF

from dmu.logging.log_store import LogStore
from dmu.stats.utilities   import print_pdf
from dmu.stats.utilities   import pdf_to_tex

log = LogStore.add_logger('dmu:tests:stats:test_utilities')
#----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:stats:utilities', 10)
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
              txt_path = 'tests/stats/utilities/print_pdf/pdf.txt')

    print_pdf(pdf,
              blind    =['sg.*', 'mu.*'],
              txt_path = 'tests/stats/utilities/print_pdf/pdf_blind.txt')

    print_pdf(pdf,
              d_const  = d_const,
              txt_path = 'tests/stats/utilities/print_pdf/pdf_const.txt')
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
