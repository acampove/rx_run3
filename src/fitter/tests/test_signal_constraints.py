'''
Module holding functions needed to test SignalConstraints class
'''

from dmu.stats import zfit
from dmu.stats import ModelFactory 
from fitter    import SignalConstraints

Loss = zfit.loss.ExtendedUnbinnedNLL
zobs = zfit.Space
# ----------------------
def _get_nll(obs : zobs) -> Loss:
    '''
    Parameters
    -------------
    obs  : Observable

    Returns
    -------------
    Likelihood with:

    - PDF used to fit signal 
    '''
    pdf_names = ['dscb'] 

    mu   = zfit.Parameter('mu', 5200, 4500, 6000)
    sg   = zfit.Parameter('sg',  150,   10, 200)
    gaus = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)

    fct  = ModelFactory(
        obs     = obs,
        l_pdf   = pdf_names,
        l_shared= [],
        l_float = [],
        preffix = 'signal')
    expo = fct.get_pdf()

    nexpo = zfit.param.Parameter('nbkg', 1000, 0, 1000_000)
    ngaus = zfit.param.Parameter('nsig', 1000, 0, 1000_000)

    bkg   = expo.create_extended(nexpo)
    sig   = gaus.create_extended(ngaus)
    pdf   = zfit.pdf.SumPDF([bkg, sig])
    dat   = pdf.create_sampler()

    return zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=dat)
# ----------------------
def test_simple() -> None:
    '''
    Simplest test
    '''
    obs = zfit.Space('dummy', limits=(4500, 6000))
    nll = _get_nll(obs = obs)    
    calc= SignalConstraints(
        name = 'signal',
        nll  = nll)

    constraints = calc.get_constraints()

    assert isinstance(constraints, list)
