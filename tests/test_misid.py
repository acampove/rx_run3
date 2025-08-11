'''
Module containing tests for MisID class
'''
import os
import pytest
import matplotlib.pyplot as plt

from dmu.stats.zfit         import zfit
from dmu.logging.log_store  import LogStore
from dmu.generic            import utilities as gut
from dmu.stats.zfit_plotter import ZFitPlotter
from zfit.interface         import ZfitPDF   as zpdf
from fitter.misid           import MisID 

log=LogStore.add_logger('fitter:test_misid')

# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    user    = os.environ['USER']
    out_dir = f'/tmp/{user}/tests/misid'
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('dmu:workflow:cache'      , 30)
    LogStore.set_level('fitter:sim_fitter'       , 30)
    LogStore.set_level('fitter:base_fitter'      , 30)
    LogStore.set_level('fitter:data_preprocessor', 10)
    LogStore.set_level('rx_misid:sample_weighter', 20)

    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------
def _validate_pdf(pdf : zpdf, name : str) -> None:
    '''
    Parameters
    -------------
    pdf : KDE to validate
    name: Needed for naming plot
    '''
    dat = pdf.create_sampler()

    obj   = ZFitPlotter(data=dat, model=pdf)
    obj.plot()
    obj.axs[0].set_title(name)
    plt.savefig(f'{Data.out_dir}/{name}.png')
# ----------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple(q2bin : str) -> None:
    '''
    Basic test for building misID component
    '''
    obs = zfit.Space('B_Mass_smr', limits=(4500, 6000))

    cfg = gut.load_conf(package='fitter_data', fpath='misid/electron/data_misid.yaml')

    obj = MisID(
        obs      = obs,
        cfg      = cfg,
        q2bin    = q2bin)
    pdf = obj.get_pdf()

    _validate_pdf(pdf=pdf, name = f'misid_in_sig_region_{q2bin}')
# ----------------------
