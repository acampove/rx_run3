'''
Module with functions needed to test ConstraintReader class
'''

import pytest
from typing                   import Final
from pathlib                  import Path
from dmu.stats.zfit           import zfit
from dmu.workflow             import Cache
from dmu                      import LogStore
from zfit.loss                import ExtendedUnbinnedNLL
from fitter.constraint_reader import ConstraintReader
from zfit                     import Space     as zobs
from zfit.param               import Parameter as zpar

log=LogStore.add_logger('fitter:test_constraint_reader')

_CONSTRAINTS : Final[list[str]] = [
    'sig_par', 
    'rare_prec_rk', 
    'rare_prec_rkst', 
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
            l_par_name = [
                'pscale_yld_Bu_Kpipiee_eq_DPC_LSFLAT']
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
    def get_params(self, floating : bool) -> set[zpar]:
        '''
        Parameters
        -------------
        floating: Bool, meant to be True

        Returns
        -------------
        Set of zfit parameters
        '''
        return self._s_par 
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('fitter:constraint_reader', 10)
# --------------------------------------------------------------
def _print_constraints(d_cns : dict[str, tuple[float,float]]) -> None:
    for name, (value, error) in d_cns.items():
        log.info(f'{name:<50}{value:<20.3f}{error:<20.3f}')
# --------------------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
@pytest.mark.parametrize('kind' , _CONSTRAINTS)
def test_simple(tmp_path : Path, kind : str, q2bin : str):
    '''
    Tests getting constraints

    Parameters
    -------------
    kind : Type of parameters
    q2bin: q2 bin
    '''

    obs = zfit.Space('dummy', limits=(4500, 6000))
    obj = Parameters(kind=kind, obs = obs)
    nll : ExtendedUnbinnedNLL = obj # type: ignore

    with Cache.cache_root(path = tmp_path):
        obj     = ConstraintReader(obj=nll, q2bin=q2bin)
        d_cns   = obj.get_constraints()
    _print_constraints(d_cns)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec':
        return

    assert len(d_cns) > 0 
# --------------------------------------------------------------
