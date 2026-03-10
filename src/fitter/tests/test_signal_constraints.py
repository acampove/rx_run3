'''
Module holding functions needed to test SignalConstraints class
'''
import pytest
from dmu       import LogStore
from dmu.stats import CorrectionImplementation, Model, zfit
from dmu.stats import ModelFactory
from rx_common import Block, Brem
from fitter    import SignalConstraints

Loss = zfit.loss.ExtendedUnbinnedNLL
zobs = zfit.Space
log  = LogStore.add_logger('fitter:test_signal_constraints')

brem_cats = [Brem.one, Brem.two]
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('fitter:signal_constraints', 10)
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
# ----------------------
@pytest.mark.parametrize('block', Block.blocks())
@pytest.mark.parametrize('brem' , brem_cats)
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

    assert len(constraints) == len(brem_cats)
    assert isinstance(constraints, list)
# ----------------------
