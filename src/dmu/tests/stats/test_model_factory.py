'''
Module containing tests for ZModel class
'''

import pytest

from dmu.stats      import zfit
from dmu            import LogStore
from dmu.stats      import print_pdf
from dmu.stats      import ModelFactory, MethodRegistry
from dmu.stats      import ParameterLibrary
from zfit.pdf       import BasePDF    as zpdf

log=LogStore.add_logger('dmu:stats:test_model_factory')
#--------------------------
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
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This runs before tests
    '''
    LogStore.set_level('dmu:stats:model_factory', 10)
# ----------------------
def _check_reparametrized_pdf(
    pdf               : zpdf,
    scales_must_float : bool) -> None:
    '''
    Parameters
    -------------
    pdf: zfit PDF
    scales_must_float: If true, assert that scales are floating
    '''
    all_params = pdf.get_params(floating = True) | pdf.get_params(floating = False)
    params     = [ par for par in all_params if par.name.endswith('_flt') ]

    assert len(params) == 2

    par_scale  = params[0] if params[0].name.endswith('scale_flt') else params[1]
    par_reso   = params[0] if params[0].name.endswith( 'reso_flt') else params[1]

    assert 'scale_flt' in par_scale.name
    assert  'reso_flt' in par_reso.name

    if scales_must_float:
        assert par_scale.floating
        assert par_reso.floating
    else:
        assert not par_scale.floating
        assert not par_reso.floating
#--------------------------
def _add_pdfs(l_pdf : list[zpdf]) -> None:
    l_par = [ zfit.param.Parameter(f'p{pdf.name}', 500, 0, 1000) for pdf in l_pdf ]

    zfit.pdf.SumPDF(l_pdf, l_par)
#--------------------------
def test_ranges():
    '''
    Test passing ranges for parameters
    '''
    l_pdf = ['cbr', 'cbl', 'dscb']
    l_shr = ['mu', 'sg']
    l_flt = ['mu', 'sg']
    ranges= {
        'mu' : [5500., 5000., 6000.],
        'sg' : [  10.,    5.,   20.],
    }

    mod   = ModelFactory(
        preffix = 'unique',
        obs     = Data.obs,
        l_pdf   = l_pdf,
        d_fix   = {'al_dscb' : 3, 'nr_dscb' : 1},
        ranges  = ranges,
        l_shared= l_shr,
        l_float = l_flt)

    pdf   = mod.get_pdf()

    print_pdf(pdf)
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
    'pdfs', [
    ['ggauss'],
    ['egauss'],
    ['qgauss'],
    ['gauss'],
    ['cbl'],
    ['cbr'],
    ['dscb'],
    ['cbl', 'cbl'],
    ['cbl', 'cbr']])
@pytest.mark.parametrize(
    'scales_must_float', 
    [True, False])
def test_rep_signal(
    scales_must_float : bool,
    pdfs              : list[str]):
    '''
    Test reparametrized signal PDFs, i.e.:
    - Use mass scales and resolutions
    - Use brem fractions

    Skipping bgauss and cbg, which have two widths

    Parameters
    -----------------
    pdfs             : List of PDF names
    scales_must_float: If true will float scales, false will float original parameters
    '''
    name  = 'reparametrized'
    log.info(f'Testing {name}')

    l_shr = ['mu', 'sg']
    l_flt = []
    d_rep = {'mu' : 'scale', 'sg' : 'reso'}

    mod   = ModelFactory(
        preffix = name,
        obs     = Data.obs,
        l_pdf   = pdfs,
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
