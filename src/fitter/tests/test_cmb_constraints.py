'''
Module used to test CmbConstraints class
'''
import pytest 

from contextlib     import ExitStack
from omegaconf      import DictConfig
from dmu.stats.zfit import zfit
from typing       import Final
from pathlib      import Path
from dmu.workflow import Cache
from dmu.stats    import ConstraintND
from rx_common    import Qsq
from fitter       import CmbConstraints
from dmu          import LogStore
from dmu.generic  import utilities           as gut
from dmu.stats    import ModelFactory
from zfit.loss    import ExtendedUnbinnedNLL as zlos
from rx_selection import selection           as sel

_COMBINATORIAL_NAME : Final[str] = 'combinatorial'

log=LogStore.add_logger('test_cmb_constraints')
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('fitter:cmb_constraints', 10)
    LogStore.set_level('dmu:stats:fitter'      , 10)
# ----------------------
def _get_nll(
    cfg   : DictConfig,
    q2bin : Qsq) -> zlos:
    '''
    Returns
    -------------
    Likelihood with a combinatorial PDF
    '''
    pdfs    = cfg.model.components['combinatorial'].categories.main.models[q2bin]
    cmb_cfg = cfg.model.components['combinatorial'][q2bin]
    obs_cfg = cfg.model.observable[q2bin]

    obs = zfit.Space(obs_cfg.name, limits=obs_cfg.range)
    mod = ModelFactory(
        preffix = 'combinatorial',
        obs     = obs,
        l_pdf   = pdfs,
        l_shared= cmb_cfg.shared,
        l_float = cmb_cfg.float,
        d_rep   = cmb_cfg.reparametrize,
        d_fix   = cmb_cfg.fix,
    )

    pdf   = mod.get_pdf()
    data  = pdf.create_sampler(1000)

    return zlos(model = pdf, data = data)
# ----------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_simple_rare(q2bin : Qsq, tmp_path : Path):
    '''
    Simplest test of CmbConstraints
    '''
    fit_cfg  = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader.yaml')
    nll      = _get_nll(cfg = fit_cfg, q2bin = q2bin)
    cuts     = {
    'cmb'   : 'mva_cmb > 0.3',
    'prc'   : 'mva_prc > 0.4',
    'brem'  : 'nbrem != 0'}

    with ExitStack() as stack:
        stack.enter_context(Cache.cache_root(path = tmp_path))
        stack.enter_context(sel.custom_selection(d_sel = cuts))

        calc      = CmbConstraints(
            name  = _COMBINATORIAL_NAME,
            kind  = '',
            nll   = nll,
            cfg   = fit_cfg,
            q2bin = q2bin)

    constraint = calc.get_constraint()

    assert isinstance(constraint, ConstraintND)

    print(constraint)
# ----------------------
@pytest.mark.parametrize('q2bin', ['jpsi'])
def test_simple_reso(q2bin : Qsq, tmp_path : Path):
    '''
    Simplest test of CmbConstraints
    '''
    fit_cfg   = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader_reso.yaml')
    nll       = _get_nll(cfg = fit_cfg, q2bin = q2bin)
    cuts      = {
        'block'   : 'block == 1',
        'cmb'     : 'mva_cmb > 0.2 && mva_cmb < 0.7',
        'prc'     : 'mva_prc > 0.5',
        'nobrm0'  : 'nbrem != 0',
        'brem_cat': 'nbrem == 1',
    }

    with ExitStack() as stack:
        stack.enter_context(Cache.cache_root(path = tmp_path))
        stack.enter_context(sel.custom_selection(d_sel = cuts))

        calc      = CmbConstraints(
            name  = _COMBINATORIAL_NAME,
            kind  = '',
            nll   = nll,
            cfg   = fit_cfg,
            q2bin = q2bin)

        constraint = calc.get_constraint()

    assert isinstance(constraint, ConstraintND)

    print(constraint)
