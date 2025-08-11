'''
Module with functions needed to test ConstraintReader class
'''

import pytest
from dmu.stats.zfit           import zfit
from dmu.logging.log_store    import LogStore
from zfit.interface           import ZfitParameter as zpar
from fitter.constraint_reader import ConstraintReader

log=LogStore.add_logger('fitter:test_constraint_reader')

# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    l_kind = [
        'sig_par', 
        'rare_prec', 
        'invalid', 
        'brem_frac']
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('fitter:constraint_reader', 10)
# --------------------------------------------------------------
class ConstraintTester:
    '''
    Class used to instantiate objects needed to test ConstraintReader
    '''
    # ----------------------
    def __init__(self, kind : str) -> None:
        '''
        Parameters
        -------------
        kind: Defines what parameters will be returning depending on test
        '''
        self._s_par = self._get_pars(kind=kind)
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
        if   kind == 'rare_prec':
            l_par_name = [
                'pscale_yld_Bd_Kstee_eq_btosllball05_DPC',
                'pscale_yld_Bu_Kstee_Kpi0_eq_btosllball05_DPC',
                'pscale_yld_Bs_phiee_eq_Ball_DPC']
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
# --------------------------------------------------------------
def _print_constraints(d_cns : dict[str, tuple[float,float]]) -> None:
    for name, (value, error) in d_cns.items():
        log.info(f'{name:<50}{value:<20.3f}{error:<20.3f}')
# --------------------------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
@pytest.mark.parametrize('kind' , ['sig_par', 'rare_prec', 'invalid', 'brem_frac'])
def test_simple(kind : str, q2bin : str):
    '''
    Tests getting constraints

    Parameters
    -------------
    kind : Type of parameters
    q2bin: q2 bin
    '''
    obj     = ConstraintTester(kind=kind)
    obj     = ConstraintReader(obj = obj, q2bin=q2bin)
    d_cns   = obj.get_constraints()
    _print_constraints(d_cns)

    # TODO: Needs to be updated when other parameter constraints be implemented
    if kind != 'rare_prec':
        return

    assert len(d_cns) > 0 
# --------------------------------------------------------------
