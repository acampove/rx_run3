'''
Module meant to hold tests for the DataPreprocessor class
'''
import os
import pytest
import matplotlib.pyplot as plt

from contextlib               import ExitStack
from pathlib                  import Path
from dmu                      import LogStore
from dmu.stats.zfit           import zfit
from dmu.generic              import utilities as gut
from dmu.stats                import utilities as sut
from dmu.workflow             import Cache
from omegaconf                import OmegaConf
from rx_selection             import selection as sel
from rx_common                import Trigger, Sample
from rx_data                  import RDFGetter
from zfit.data                import Data      as zdata
from fitter                   import DataPreprocessor

log=LogStore.add_logger('fitter:test_data_preprocessor')
# -------------------------------------------------
class Data:
    '''
    Meant to hold shared attributes
    '''
    user    = os.environ['USER']
# -------------------------------------------------
@pytest.mark.fixture(autouse=True)
def max_entries():
    '''
    This fixture runs for all the tests in this module
    '''
    with RDFGetter.max_entries(value=100_000):
        yield
# -------------------------------------------------
def _validate_data(
    data     : zdata, 
    name     : Path,
    tmp_path : Path) -> None:
    '''
    Makes validation plots from zfit data
    '''
    plt_path = tmp_path / f'{name}.png'

    arr_data = data.numpy()
    rng      = sut.range_from_obs(obs=data.space)
    if data.weights is None:
        arr_wgt = None
    else:
        arr_wgt = data.weights.numpy()

    log.info(f'Saving to: {plt_path}')
    plt.hist(arr_data, histtype='step', bins=100, range=rng, weights=arr_wgt)
    plt.savefig(plt_path)
    plt.close()
# -------------------------------------------------
@pytest.mark.parametrize('sample', [Sample.bpkpee])
def test_mc(tmp_path : Path, sample : Sample):
    '''
    Tests class with toys
    '''
    obs = zfit.Space('B_Mass', limits=(5180, 6000))
    cuts= {
        'q2' : '(1)',
        'cmb': 'mva_cmb > 0.85',
        'prc': 'mva_prc > 0.50',
        'mass': 'B_Mass > 4500 && B_Mass < 6000',
        'nobrm0': 'nbrem != 0',
        'brem_cat': 'nbrem == 2',
        'block': 'block == 1',
    }

    out_dir = Path(sample)
    with ExitStack() as stack:
        stack.enter_context(Cache.cache_root(path = tmp_path))
        stack.enter_context(sel.custom_selection(d_sel = cuts))

        prp = DataPreprocessor(
            obs    = obs,
            out_dir= out_dir,
            sample = sample,
            trigger= Trigger.rk_ee_os,
            wgt_cfg= None,
            q2bin  = 'jpsi')
        dat = prp.get_data()

    _validate_data(data=dat, name=out_dir, tmp_path = tmp_path)
# -------------------------------------------------
@pytest.mark.parametrize('sample', [
    Sample.data_24,
    Sample.bpkpjpsimm])
def test_muon_data(tmp_path : Path, sample : Sample):
    '''
    Tests class with toys
    '''
    obs = zfit.Space('B_Mass', limits=(5180, 6000))
    name= Path(f'data_preprocessor/{sample}_muon_data')

    with Cache.cache_root(path = tmp_path):
        prp = DataPreprocessor(
            obs    = obs,
            out_dir= name,
            sample = sample,
            trigger= Trigger.rk_mm_os,
            wgt_cfg= None,
            q2bin  = 'jpsi')
        dat = prp.get_data()

    _validate_data(data=dat, name=name, tmp_path = tmp_path)
# -------------------------------------------------
@pytest.mark.parametrize('sample', [
    Sample.data_24,
    Sample.bpkpjpsiee])
@pytest.mark.parametrize('brem_cat', [0, 1, 2])
def test_brem_cat_data(
    tmp_path : Path, 
    sample   : Sample, 
    brem_cat : int):
    '''
    Tests class with toys
    '''
    obs = zfit.Space('B_Mass', limits=(4500, 6000))
    name= Path(f'data_preprocessor/{sample}_brem_{brem_cat:03}')
    cut = {'brem' : f'nbrem == {brem_cat}'}

    with Cache.cache_root(path = tmp_path):
        prp = DataPreprocessor(
            obs    = obs,
            out_dir= name,
            sample = sample,
            trigger= Trigger.rk_ee_os,
            cut    =  cut, 
            wgt_cfg= None,
            q2bin  = 'jpsi')
        dat = prp.get_data()

    _validate_data(data=dat, name=name, tmp_path = tmp_path)
# -------------------------------------------------
@pytest.mark.skip(reason='These tests require smear friend trees for noPID samples')
@pytest.mark.parametrize('sample', [
    Sample.bpkkk,
    Sample.bpkpipi])
@pytest.mark.parametrize('region', ['kpipi' ,     'kkk'])
@pytest.mark.parametrize('kind'  , ['signal', 'control'])
def test_with_pid_weights(
    tmp_path : Path,
    sample   : Sample,
    region   : str, 
    kind     : str) -> None:
    '''
    Parameters
    -------------
    sample: MC sample, weights are not applied to data
    region: Kind of control region
    kind  : Either 'signal' or 'control'
    '''
    cfg_spl = gut.load_conf(package='fitter_data', fpath='model/weights/splitting.yaml')
    cfg_wgt = gut.load_conf(package='fitter_data', fpath='model/weights/weights.yaml')

    wgt_cfg = {'PID' : {
           'splitting' : cfg_spl,
           'weights'   : cfg_wgt}}

    obs     = zfit.Space(f'B_Mass_{region}', limits=(4500, 6000))
    wgt_cfg = OmegaConf.create(wgt_cfg)
    cut     = {
        'brem' : 'nbrem == 1',
        'pid_l': '(1)'}

    name = Path(f'data_preprocessor/with_pid_weights_{sample}_{region}_{kind}')

    with Cache.cache_root(path = tmp_path):
        prp  = DataPreprocessor(
            obs    = obs,
            out_dir= name,
            sample = sample,
            trigger= Trigger.rk_ee_nopid,
            cut    = cut, 
            wgt_cfg= wgt_cfg,
            is_sig = kind == 'signal',
            q2bin  = 'jpsi')
        dat  = prp.get_data()

    _validate_data(data=dat, name=name, tmp_path = tmp_path)
# -------------------------------------------------
