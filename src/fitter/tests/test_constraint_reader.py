'''
Module with functions needed to test ConstraintReader class
'''
import pytest
import zfit

from typing       import Final
from pathlib      import Path
from dmu.workflow import Cache
from dmu          import LogStore
from dmu.stats    import print_constraints
from rx_common    import Qsq
from zfit.loss    import ExtendedUnbinnedNLL
from zfit.param   import Parameter           as zpar
from zfit         import Space               as zobs
from fitter       import ConstraintReader

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
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
@pytest.mark.parametrize('kind' , _CONSTRAINTS)
def test_simple(tmp_path : Path, kind : str, q2bin : Qsq):
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
        obj         = ConstraintReader(obj=nll, q2bin=q2bin)
        constraints = obj.get_constraints()

    print_constraints(constraints = constraints)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec_rk':
        return

    assert len(constraints) > 0 
# --------------------------------------------------------------
