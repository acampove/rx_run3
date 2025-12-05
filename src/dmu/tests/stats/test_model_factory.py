'''
Module containing tests for ZModel class
'''

import pytest

from dmu.stats.zfit          import zfit
from zfit.pdf                import BasePDF    as zpdf
from dataclasses             import dataclass
from dmu.stats.utilities     import print_pdf
from dmu                     import LogStore
from dmu.stats.model_factory import ModelFactory, MethodRegistry
from dmu.stats.parameters    import ParameterLibrary

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
def _add_pdfs(l_pdf : list[zpdf]) -> None:
    l_par = [ zfit.param.Parameter(f'p{pdf.name}', 1000, 0, 1000) for pdf in l_pdf ]

    pdf = zfit.pdf.SumPDF(l_pdf, l_par)
#--------------------------
def test_fix_params():
    '''
    Will test fixing subset of parameters
    '''
    l_pdf = ['cbr', 'cbl', 'dscb']
    l_shr = ['mu', 'sg']
    l_flt = ['mu', 'sg']

    mod   = ModelFactory(
        preffix = 'unique',
        obs     = Data.obs,
        l_pdf   = l_pdf,
        d_fix   = {'al_dscb' : 3, 'nr_dscb' : 1},
        l_shared= l_shr,
        l_float = l_flt)

    pdf   = mod.get_pdf()

    print_pdf(pdf)
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
def test_all_pdf(name : str):
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
@pytest.mark.parametrize('kind', ['cbr', 'cbl', 'dscb', 'gauss'])
def test_override_parameter(kind: str):
    '''
    Will create a PDF by overriding parameters
    '''
    log.info(f'Testing {kind}')

    l_shr = ['mu', 'sg']
    l_flt = ['mu']

    ParameterLibrary.print_parameters(kind=kind)

    with ParameterLibrary.values(parameter='mu', kind=kind, val=3100, low=2200, high=3500),\
         ParameterLibrary.values(parameter='sg', kind=kind, val=  30, low=  30, high=  30):

        mod   = ModelFactory(
            preffix = kind,
            obs     = Data.obs,
            l_pdf   = [kind],
            l_shared= l_shr,
            l_float = l_flt)
        pdf   = mod.get_pdf()

    s_par = pdf.get_params(floating=False)
    [sg]  = [ par for par in s_par if par.name == f'sg_{kind}' ]

    assert sg.floating is False

    print_pdf(pdf)
#--------------------------
def test_shared_parameters():
    '''
    Will create a PDF for each of the models supported
    '''

    l_shr = ['mu', 'sg']
    l_flt = ['mu', 'sg']

    mu = zfit.param.Parameter('mu_flt', 5280, 5000, 5500)
    sg = zfit.param.Parameter('sg_flt',   80,   20,  100)

    mod_1 = ModelFactory(
        preffix = 'cbl',
        obs     = Data.obs,
        l_pdf   = ['cbl'],
        l_shared= l_shr,
        l_reuse = [mu, sg],
        l_float = l_flt)
    pdf_1 = mod_1.get_pdf()

    mod_2 = ModelFactory(
        preffix = 'cbr',
        obs     = Data.obs,
        l_pdf   = ['cbr'],
        l_shared= l_shr,
        l_reuse = [mu, sg],
        l_float = l_flt)
    pdf_2 = mod_2.get_pdf()

    print_pdf(pdf_1)
    print_pdf(pdf_2)

    _add_pdfs(l_pdf=[pdf_1, pdf_2])
#--------------------------
