'''
This file is needed by pytest
'''
import os
os.environ["CUDA_VISIBLE_DEVICES"] = '-1'
from pathlib import Path
from typing  import cast

import mplhep
import matplotlib.pyplot as plt
import pandas            as pnd
import pytest

from rx_selection                import selection  as sel
from rx_efficiencies.decay_names import DecayNames as dn
from _pytest.config              import Config

from dmu.stats.zfit              import zfit
from dmu.workflow.cache          import Cache
from dmu.logging.log_store       import LogStore
from zfit.interface              import ZfitParameter as zpar
from zfit.interface              import ZfitSpace     as zobs

executed_tests = set()
log = LogStore.add_logger('rx_efficiencies:conftest')

# --------------------------------------------------------------
def pytest_addoption(parser):
    parser.addoption('--ntoys', type=int, default=-1, help='Override number of toys, by default will do what is in the config')

@pytest.fixture
def ntoys(request):
    return request.config.getoption('--ntoys')

@pytest.fixture(scope='session')
def test_dir() -> Path:
    user     = os.environ.get('USER', 'unknown')
    dir_path = Path(f'/tmp/{user}/tests/fitter')
    dir_path.mkdir(parents=True, exist_ok=True)

    return dir_path
# --------------------------------------------------------------
class TestingParametersHolder:
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
        elif kind == 'rare_prec':
            l_par_name = [
                'pscale_yld_Bd_Kstee_eq_btosllball05_DPC',
                'pscale_yld_Bu_Kstee_Kpi0_eq_btosllball05_DPC',
                'pscale_yld_Bs_phiee_eq_Ball_DPC']
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
# -----------------------------------
class ScalesData:
    '''
    data class
    '''
    df_def_wp = pnd.DataFrame(columns=['Process', 'mva_cut', 'Q2', 'Value', 'Error'])
    df_mva_wp = pnd.DataFrame(columns=['Process', 'mva_cut', 'Q2', 'Value', 'Error'])

    plt.style.use(mplhep.style.LHCb2)
    # ------------------
    @staticmethod
    def collect_def_wp(proc : str, mvawp: str, q2bin : str, value : float, error : float) -> None:
        '''
        Picks test outputs and uses it to fill dataframe
        '''
        size                           = len(ScalesData.df_def_wp)
        ScalesData.df_def_wp.loc[size] = [proc, mvawp, q2bin, value, error]
    # ------------------
    @staticmethod
    def collect_mva_wp(proc : str, mvawp: str, q2bin : str, value : float, error : float) -> None:
        '''
        Picks test outputs and uses it to fill dataframe
        '''
        size                           = len(ScalesData.df_mva_wp)
        ScalesData.df_mva_wp.loc[size] = [proc, mvawp, q2bin, value, error]
    # ------------------
    @staticmethod
    def plot_scales_def_wp():
        '''
        Plots scales from dataframe with default WP
        '''
        df      = ScalesData.df_def_wp
        mva_cut = df.mva_cut.iloc[0]

        plt.figure(figsize=(15,10))
        for proc, df_proc in df.groupby('Process'):
            proc = cast(str, proc)
            if proc == 'bpkpee':
                continue

            decay = dn.tex_from_decay(decay=proc)
            plt.plot(df_proc.Q2, df_proc.Value, label=decay)
            plt.fill_between(df_proc.Q2, df_proc.Value - df_proc.Error, df_proc.Value + df_proc.Error, alpha=0.2)

        user    = os.environ['USER']
        out_dir = f'/tmp/{user}/tests/fitter'
        out_dir = f'{out_dir}/prec_scales/plots/prec_scales'
        os.makedirs(out_dir, exist_ok=True)

        log.debug(f'Sending plots to: {out_dir}')

        plt.grid()
        plt.title(mva_cut)
        plt.legend()
        plt.ylim(0.0, 0.15)
        plt.xlabel('')
        plt.ylabel(r'$N_{PRec}/N_{Signal}$')
        plt.savefig(f'{out_dir}/scales_def_wp.png')
        plt.close()
    # ------------------
    @staticmethod
    def plot_scales_mva_wp(df_mva : pnd.DataFrame, q2bin : str):
        '''
        Plots scales from dataframe with scanned MVA WP
        '''
        df_mva['mva_cut'] = df_mva.mva_cut.str.replace('mva_', '')
        df_mva['mva_cut'] = df_mva.mva_cut.str.replace('&&'  , '')
        df_mva['mva_cut'] = df_mva.mva_cut.str.replace('('   , '')
        df_mva['mva_cut'] = df_mva.mva_cut.str.replace(')'   , '')

        plt.figure(figsize=(30,20))
        for process, df in df_mva.groupby('Process'):
            process= cast(str, process)
            decay  = dn.tex_from_decay(process)
            plt.plot(df.mva_cut, df.Value, label=decay)
            plt.fill_between(df.mva_cut, df.Value - df.Error, df.Value + df.Error, alpha=0.2)

        user    = os.environ['USER']
        out_dir = f'/tmp/{user}/tests/fitter'
        out_dir = f'{out_dir}/prec_scales/plots/prec_scales'
        os.makedirs(out_dir, exist_ok=True)

        plt.legend()
        plt.title(q2bin)
        plt.grid()
        plt.ylim(0.0, 0.15)
        plt.xlabel('')
        plt.xticks(rotation=70)
        plt.ylabel(r'$N_{PRec}/N_{Signal}$')
        plt.savefig(f'{out_dir}/scales_mva_wp_{q2bin}.png')
        plt.close()
# ----------------------------------------
def _set_logs() -> None:
    LogStore.set_level('fitter:data_model'          , 10)
    LogStore.set_level('fitter:sim_fitter'          , 10)
    LogStore.set_level('fitter:data_fitter'         , 10)
    LogStore.set_level('fitter:base_fitter'         , 10)
    LogStore.set_level('fitter:data_preprocessor'   , 10)
    LogStore.set_level('fitter:prec'                , 10)
    LogStore.set_level('fitter:prec_scales'         , 10)
    LogStore.set_level('rx_fitter:constraint_reader', 10)

    # Silence what is below

    LogStore.set_level('rx_efficiencies:efficiency_calculator', 30)
    LogStore.set_level('rx_selection:selection'               , 30)
    LogStore.set_level('rx_selection:truth_matching'          , 30)
    LogStore.set_level('rx_data:path_splitter'                , 30)
    LogStore.set_level('rx_data:rdf_getter'                   , 30)
    LogStore.set_level('dmu:workflow:cache'                   , 30)
    LogStore.set_level('dmu:stats:model_factory'              , 30)
    LogStore.set_level('dmu:zfit_plotter'                     , 30)

    os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
    os.environ['GRPC_VERBOSITY'] = 'ERROR'
# ----------------------------------------
def pytest_configure(config : Config):
    '''
    This function is called before any test run
    '''
    _config = config

    # Use this as the root directory where everything gets
    # cached

    user      = os.environ['USER']
    cache_dir = f'/tmp/{user}/tests/fitter'
    Cache.set_cache_root(root=cache_dir)

    _set_logs()

    plt.style.use(mplhep.style.LHCb2)
# ----------------------------------------
@pytest.fixture
def skip_mass_cut():
    '''
    This is a fixture meant to be passed as an argumen to tests
    It will ensure that the test is ran with data without the mass cut
    '''
    with sel.custom_selection(d_sel = {'mass' : '(1)'}):
        yield
# ----------------------
@pytest.fixture
def get_parameters_holder() -> type[TestingParametersHolder]:
    '''
    Returns
    -------------
    Returns TestingParametersHolder class (not instance)
    '''
    return TestingParametersHolder
# -----------------------------------
def pytest_runtest_logreport(report):
    '''
    Will collect the names (?) of the tests that were ran and passed
    in the executed_tests set
    '''
    if report.when == "call" and report.passed:
        executed_tests.add(report.nodeid)
# -----------------------------------
def pytest_sessionfinish():
    '''
    Runs at the end
    '''
    if any('test_all_datasets' in test for test in executed_tests):
        ScalesData.plot_scales_def_wp()

    if any('test_seq_scan_scales' in test for test in executed_tests):
        for q2bin, df_q2 in ScalesData.df_mva_wp.groupby('Q2'):
            q2bin = cast(str, q2bin)
            ScalesData.plot_scales_mva_wp(df_mva = df_q2, q2bin=q2bin)
# -----------------------------------
