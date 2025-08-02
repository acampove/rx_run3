'''
Module used to test DataFitter
'''

from dmu.stats.zfit        import zfit
from dmu.stats             import utilities as sut
from dmu.logging.log_store import LogStore
from fitter.data_fitter    import DataFitter

log=LogStore.add_logger('fitter:test_data_fitter')
# ----------------------
def test_single_region() -> None:
    '''
    Test fitting with single signal region
    '''
    pdf = sut.get_model(kind='s+b')
    dat = pdf.create_sampler(10_000)
    nll = zfit.loss.ExtendedBinnedNLL(data=dat, model=pdf)

    d_nll = {'signal' : nll}

    ftr = DataFitter(d_nll=d_nll, cfg={})
    ftr.run()
