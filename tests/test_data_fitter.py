'''
Module used to test DataFitter
'''

from omegaconf             import OmegaConf
from dmu.stats.zfit        import zfit
from dmu.stats             import utilities as sut
from dmu.generic           import utilities as gut
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
    nll = zfit.loss.ExtendedUnbinnedNLL(data=dat, model=pdf)

    sel_cfg = {'default' : {}, 'fit' : {}}
    sel_cfg = OmegaConf.create(obj=sel_cfg)
    d_nll   = {'signal_region' : (nll, sel_cfg)}

    cfg = gut.load_conf(package='fitter_data', fpath='tests/single_region.yaml')
    ftr = DataFitter(d_nll=d_nll, cfg=cfg)
    ftr.run()
# ----------------------
