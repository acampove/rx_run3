'''
Module holding functions needed to test SignalConstraints class
'''
import pytest
from dmu       import LogStore
from dmu.stats import CorrectionImplementation, Model, print_pdf, zfit
from dmu.stats import ModelFactory
from rx_common import Block, Brem
from fitter    import SignalConstraints
from fitter    import Category 
from fitter    import CategoryMerger

Loss = zfit.loss.ExtendedUnbinnedNLL
zobs = zfit.Space
zpdf = zfit.pdf.BasePDF
log  = LogStore.add_logger('fitter:test_signal_constraints')

_BREM_CATS = [Brem.one, Brem.two]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('fitter:signal_constraints', 10)
# ----------------------
def _fix_tails(pdf : zpdf) -> zpdf:
    '''
    Parameters
    -------------
    pdf: Zfit PDF

    Returns
    -------------
    Input PDF with tails fixed
    '''
    pars = pdf.get_params()
    for par in pars:
        if par.name.startswith('mu_'):
            continue

        if par.name.startswith('sg_'):
            continue

        par.floating = False
   
    return pdf
# ----------------------
def _get_signal(
    obs  : zobs,
    brem : Brem, 
    block: Block):

    signal_name = f'signal_brem_{brem}_b{block}'
    scale       = CorrectionImplementation.scale
    reso        = CorrectionImplementation.reso

    with ModelFactory.correction(fixed = False):
        fct = ModelFactory(
            obs     = obs,
            l_pdf   = [Model.dscb],
            l_shared= [],
            l_float = [],
            d_rep   = {'mu' : scale, 'sg' : reso},
            preffix = signal_name)
        dscb = fct.get_pdf()
        dscb = _fix_tails(pdf = dscb)

    return dscb
# ----------------------
def _get_nll(
    obs   : zobs,
    brem  : Brem,
    block : Block) -> Loss:
    '''
    Parameters
    -------------
    obs  : Observable

    Returns
    -------------
    Likelihood with:

    - PDF used to fit signal
    '''
    dscb= _get_signal(
        obs  = obs,
        brem = brem, 
        block= block)

    fct  = ModelFactory(
        obs     = obs,
        l_pdf   = [Model.exp],
        l_shared= [],
        l_float = [],
        preffix = 'bkg')
    expo = fct.get_pdf()

    nbkg  = zfit.param.Parameter('nbkg', 1000, 0, 1000_000)
    nsig  = zfit.param.Parameter('nsig', 1000, 0, 1000_000)

    bkg   = expo.create_extended(nbkg)
    sig   = dscb.create_extended(nsig)
    pdf   = zfit.pdf.SumPDF([bkg, sig])
    dat   = pdf.create_sampler()

    return zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=dat)
# ----------------------------------------
def _get_category(
    block: Block,
    brem : Brem,
    obs  : zobs) -> Category:
    '''
    Parameters
    -------------

    Returns
    -------------
    Category object
    '''

    pdf  = _get_signal(
        obs  = obs,
        brem = brem, 
        block= block)

    cat  = Category(
        name      = f'brem_{brem}_b{block}', 
        pdf       = pdf, 
        sumw      = 1000., 
        selection = {'mass' : '(1)'},
        model     = ['gauss'])

    return cat
# ----------------------
@pytest.mark.parametrize('block', Block.blocks())
@pytest.mark.parametrize('brem' , _BREM_CATS)
def test_simple(block : Block, brem : Brem) -> None:
    '''
    Simplest test
    '''
    obs = zfit.Space('dummy', limits=(4500, 6000))
    nll = _get_nll(
        obs   = obs,
        block = block,
        brem  = brem)

    calc= SignalConstraints(nll = nll)
    constraints = calc.get_constraints()

    for cns in constraints:
        log.info(cns)

    assert len(constraints) == len(_BREM_CATS)
    assert isinstance(constraints, list)
# ----------------------
def test_add_brem_block():
    '''
    Add brems and then blocks
    '''
    obs        = zfit.Space('dummy', limits=(4500, 6000))
    categories = [ _get_category(block = block, brem = brem, obs = obs) for block in Block.blocks() for brem in _BREM_CATS ]

    mgr = CategoryMerger(categories = categories)
    cat = mgr.get_category()

    print_pdf(cat.pdf)
# ----------------------
