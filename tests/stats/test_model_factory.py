'''
Module containing tests for ZModel class
'''

from dataclasses import dataclass

import zfit
import pytest
from dmu.stats.utilities     import print_pdf
from dmu.logging.log_store   import LogStore
from dmu.stats.model_factory import ModelFactory, MethodRegistry

log=LogStore.add_logger('dmu:stats:test_model_factory')
#--------------------------
@dataclass
class Data:
    '''
    Data class used to share
    '''
    obs = zfit.Space('mass', limits=(5080, 5680))

    l_arg_run3 = [
            ('2024', 'ETOS', 'ctrl'),
            ]

    l_arg_simple = [
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'MTOS', 'psi2'),
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ]

    l_arg_syst = [
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'ETOS', 'ctrl'),
            ]

    l_arg_prc = [
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ]

    l_arg_signal = [
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'MTOS', 'psi2'),
            ]
#--------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:stats:model_factory', 10)
#--------------------------
def test_unique_pdf():
    '''
    Will test only signal builder
    '''
    l_pdf = ['cbr', 'cbl', 'dscb']
    l_shr = ['mu', 'sg']
    l_flt = ['mu', 'sg']
    mod   = ModelFactory(preffix = 'unique',
                         obs     = Data.obs,
                         l_pdf   = l_pdf,
                         l_shared= l_shr,
                         l_float = l_flt)

    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
def test_repeated_pdf():
    '''
    Will test only signal builder
    '''
    log.info('Testing for repeated PDFs')

    l_pdf = ['cbr'] + 2 * ['cbl']
    l_shr = ['mu', 'sg']
    l_flt = ['mu']
    mod   = ModelFactory(
            preffix = 'repeated',
            obs     = Data.obs,
            l_pdf   = l_pdf,
            l_shared= l_shr,
            l_float = l_flt)
    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
@pytest.mark.parametrize('name', MethodRegistry.get_pdf_names())
def test_all_pdf(name : list[str]):
    '''
    Will create a PDF for each of the models supported
    '''
    log.info(f'Testing {name}')

    l_shr = ['mu', 'sg']
    l_flt = ['mu', 'sg']

    mod   = ModelFactory(
            preffix = name,
            obs     = Data.obs,
            l_pdf   = [name],
            l_shared= l_shr,
            l_float = l_flt)
    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
@pytest.mark.parametrize(
        'l_name',
        [
            ['gauss'],
            ['cbl'],
            ['cbr'],
            ['dscb'],
            ['cbl', 'cbl'],
            ['cbl', 'cbr']])
def test_rep_signal(l_name : list[str]):
    '''
    Test reparametrized signal PDFs, i.e.:
    - Use mass scales and resolutions
    - Use brem fractions
    '''
    name  = 'reparametrized'
    log.info(f'Testing {name}')

    l_shr = ['mu', 'sg']
    l_flt = []
    d_rep = {'mu' : 'scale', 'sg' : 'reso'}

    mod   = ModelFactory(
            preffix = name,
            obs     = Data.obs,
            l_pdf   = l_name,
            d_rep   = d_rep,
            l_shared= l_shr,
            l_float = l_flt)
    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
