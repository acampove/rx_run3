'''
Module with functions needed to test ConstraintReader class
'''
import pytest

from typing         import Final, cast
from pathlib        import Path
from dmu.workflow   import Cache
from dmu            import LogStore
from dmu.stats      import ModelFactory, print_constraints
from dmu.stats.zfit import zfit
from dmu.generic    import utilities           as gut
from rx_common      import Qsq
from fitter         import FitConfig
from zfit.loss      import ExtendedUnbinnedNLL
from zfit.param     import Parameter           as zpar
from zfit           import Space               as zobs
from fitter         import ConstraintReader
from zfit.core.loss import ZfitParameter

log=LogStore.add_logger('fitter:test_constraint_reader')

_COMBINATORIAL_NAME : Final[str]       = 'combinatorial'
_CONSTRAINTS        : Final[list[str]] = [
    'sig_par', 
    'rare_prec_rk', 
    'invalid', 
    'brem_frac']
# ----------------------
class Parameters:
    '''
    Class used to instantiate objects holding parameters and observables
    They are needed in tests
    '''
    # ----------------------
    def __init__(self, kind : str, obs : zobs) -> None:
        '''
        Parameters
        -------------
        kind: Defines what parameters will be returning depending on test
        obs : Observable
        '''
        self._obs   = obs
        self._s_par = self._get_pars(kind=kind)
    # ----------------------
    @property
    def space(self) -> zobs:
        '''
        Returns
        -------------
        Observable
        '''
        return self._obs
    # ----------------------
    def _get_pars(self, kind : str) -> set[zpar]:
        '''
        Parameters
        -------------
        kind: Type of parameters

        Returns
        -------------
        Set of zfit parameters
        '''
        if   kind == 'dummy':
            return set()
        elif kind == 'rare_prec_rk':
            l_par_name = [
                'pscale_yld_Bd_Kstee_eq_btosllball05_DPC',
                'pscale_yld_Bu_Kstee_Kpi0_eq_btosllball05_DPC',
                'pscale_yld_Bs_phiee_eq_Ball_DPC']
        elif kind == 'rare_prec_rkst':
            l_par_name = []
        elif kind == 'rare_misid':
            l_par_name = [
                'yld_kpipi',
                'yld_kkk']
        elif kind == 'brem_frac':
            l_par_name = [
                'frac_brem_000',
                'frac_brem_001',
                'frac_brem_002']
        elif kind == 'sig_par':
            l_par_name = [
                'ar_dscb_Signal_002_1_reso_flt',
                'mu_Signal_000_scale_flt',
                'mu_Signal_001_scale_flt',
                'mu_Signal_002_scale_flt',
                'nl_dscb_Signal_001_1_reso_flt',
                'nr_dscb_Signal_002_1_reso_flt',
                'sg_Signal_000_reso_flt',
                'sg_Signal_001_reso_flt',
                'sg_Signal_002_reso_flt',
            ]
        elif kind == 'invalid':
            l_par_name = [
                'ap_hypexp',
                'bt_hypexp',
                'mu_hypexp',
                'ncmb',
                'nsig']
        else:
            raise ValueError(f'Invalid kind of parameters: {kind}')

        return { zfit.Parameter(name, 0, 0, 1) for name in l_par_name }
    # ----------------------
    def get_params(self, floating : bool) -> set[zpar] | set[ZfitParameter]:
        '''
        Parameters
        -------------
        floating: Bool, meant to be True

        Returns
        -------------
        Set of zfit parameters
        '''
        _ = floating

        return self._s_par 
# ----------------------
def _get_nll(
    obs  : zobs, 
    q2bin: Qsq,
    cfg  : FitConfig) -> ExtendedUnbinnedNLL:
    '''
    Parameters
    -------------
    obs  : Observable
    cfg  : Configuration for fit
    q2bin: E.g. central

    Returns
    -------------
    Likelihood with:

    - PDF used to fit combinatorial
    - Parameters needed for misID
    '''
    pdf_names = cfg.fit_cfg.model.components.combinatorial.categories.main.models[q2bin]

    mu   = zfit.Parameter('mu', 5200, 4500, 6000)
    sg   = zfit.Parameter('sg',  150,   10, 200)
    gaus = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)

    fct  = ModelFactory(
        obs     = obs,
        l_pdf   = pdf_names,
        l_shared= [],
        l_float = [],
        preffix = 'combinatorial')
    expo = fct.get_pdf()

    nexpo = zfit.param.Parameter('nbkg', 1000, 0, 1000_000)
    ngaus = zfit.param.Parameter('nsig', 1000, 0, 1000_000)

    bkg   = expo.create_extended(nexpo)
    sig   = gaus.create_extended(ngaus)
    pdf   = zfit.pdf.SumPDF([bkg, sig])
    dat   = pdf.create_sampler()

    return zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=dat)
# ----------------------
@pytest.fixture(scope='module', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('fitter:constraint_reader', 10)
    LogStore.set_level('fitter:cmb_constraints'  , 10)
# --------------------------------------------------------------
def _get_fit_config(q2bin : Qsq) -> FitConfig:
    '''
    Parameters
    -------------
    q2bin: E.g. central

    Returns
    -------------
    Object storing fit configuration
    '''
    fit_cfg = gut.load_conf(package='fitter_data', fpath = 'tests/fits/constraint_reader.yaml')

    return FitConfig(
        name    = 'test',
        group   = 'test',
        fit_cfg = fit_cfg, 
        mva_cmb = 0.0,
        mva_prc = 0.0,
        q2bin   = q2bin,
    )
# --------------------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
@pytest.mark.parametrize('kind' , _CONSTRAINTS)
def test_all_but_cmb(
    tmp_path : Path, 
    kind     : str, 
    q2bin    : Qsq,
    slow_mode: bool):
    '''
    Tests all the constraints but the combinatorial shape

    Parameters
    -------------
    kind : Type of parameters
    q2bin: q2 bin
    '''

    obs = zfit.Space('dummy', limits=(4500, 6000))
    nll = Parameters(kind = kind, obs = obs) 
    nll = cast(ExtendedUnbinnedNLL, nll) # Tests will only need get_params

    cfg = _get_fit_config(q2bin = q2bin)
    del cfg.fit_cfg.model.components[_COMBINATORIAL_NAME]

    if not slow_mode:
        log.info('Skipping misid constraints')
        del cfg.fit_cfg.model.components['kkk']
        del cfg.fit_cfg.model.components['kpipi']
    else:
        log.info('Running full test')

    with Cache.cache_root(path = tmp_path):
        obj         = ConstraintReader(nll=nll, cfg=cfg)
        constraints = obj.get_constraints()

    print_constraints(constraints = constraints)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec_rk':
        return

    assert len(constraints) > 0 
# --------------------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_only_cmb(
    tmp_path : Path, 
    q2bin    : Qsq):
    '''
    Tests all the constraints but the combinatorial shape

    Parameters
    -------------
    q2bin: q2 bin
    '''

    obs = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    cfg = _get_fit_config(q2bin = q2bin)
    nll = _get_nll(obs=obs, cfg=cfg, q2bin = q2bin) 
    del cfg.fit_cfg.model.components['kkk']
    del cfg.fit_cfg.model.components['kpipi']

    with Cache.cache_root(path = tmp_path):
        obj         = ConstraintReader(nll=nll, cfg=cfg)
        constraints = obj.get_constraints()

    print_constraints(constraints = constraints)

    assert len(constraints) > 0 
# --------------------------------------------------------------
