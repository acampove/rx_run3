'''
This file is needed by pytest
'''
import os
from typing import cast

import mplhep
import matplotlib.pyplot as plt
import pandas            as pnd
import pytest

from rx_selection                import selection  as sel
from rx_efficiencies.decay_names import DecayNames as dn
from _pytest.config              import Config

from dmu.workflow.cache          import Cache
from dmu.logging.log_store       import LogStore

executed_tests = set()
log = LogStore.add_logger('rx_efficiencies:conftest')
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

        out_dir = 'plots/prec_scales'
        os.makedirs(out_dir, exist_ok=True)

        log.warning(f'Sending plots to: {out_dir}')

        plt.grid()
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
            decay = dn.tex_from_decay(process)
            plt.plot(df.mva_cut, df.Value, label=decay)
            plt.fill_between(df.mva_cut, df.Value - df.Error, df.Value + df.Error, alpha=0.2)

        out_dir = 'plots/prec_scales'
        os.makedirs(out_dir, exist_ok=True)

        plt.legend()
        plt.title(q2bin)
        plt.grid()
        plt.ylim(0.0, 0.40)
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

    user = os.environ['USER']
    Cache.set_cache_root(root=f'/tmp/{user}/tests/fitter')
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
            ScalesData.plot_scales_mva_wp(df_mva = df_q2, q2bin=q2bin)
# -----------------------------------
