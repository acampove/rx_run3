'''
Module storing data classes needed by tests
Also pytest functions intended to be run after tests
'''
import os

import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from dmu.workflow.cache    import Cache     as Wcache

# -----------------------------------
class DataCollector:
    '''
    Class meant to:

    - Collect outputs of tests into dataframes
    '''
    d_df = {}
    out_dir = '/tmp/tests/rx_misid/mc_scale'
    # ---------------------------------------
    @staticmethod
    def add_entry(name : str, data : dict):
        '''
        Takes:

        name: Identifier for test, used as key of dictionary of dataframes
        data: Dictionary mapping name of quantity to its value, needed to add row to dataframe
        '''
        if name not in DataCollector.d_df:
            columns = list(data)
            DataCollector.d_df[name] = pnd.DataFrame(columns=columns)

        df=DataCollector.d_df[name]
        df.loc[len(df)] = data
# -----------------------------------
def _plot_scales(df : pnd.DataFrame, name : str, kind : str) -> None:
    ax = None
    for sample, df_sample in df.groupby('sample'):
        ax = df_sample.plot(x='q2bin', y=kind, label=sample, ax=ax)

    out_dir = f'{DataCollector.out_dir}/{name}'
    os.makedirs(out_dir, exist_ok=True)

    if   kind == 'ctr_dt':
        plt.title('Estimate of signal and leakage in MisID region')
        plt.ylabel(r'Number of Candidates')
    elif kind == 'scale':
        plt.title('MC to Data MisID Scale factor')
        plt.ylabel(r'Ratio of yields: $N_{x}^{Data}/N_{x}^{MC}$')
    else:
        raise ValueError(f'Invalid kind: {kind}')

    plt.xlabel(r'$q^2$')
    plt.grid()
    plt.savefig(f'{out_dir}/{kind}.png')
    plt.close()
# -----------------------------------
def pytest_sessionstart():
    '''
    This will run before any test
    '''
    plt.style.use(mplhep.style.LHCb2)
    _set_logs()
    Wcache.set_cache_root(root='/tmp/misid/cache')

    gut.TIMER_ON = True
# -----------------------------------
def pytest_sessionfinish():
    '''
    Runs at the end
    '''
    if 'simple' in DataCollector.d_df:
        df = DataCollector.d_df['simple']
        df['ctr_dt'] = df['scale'] * df['ctr_mc']

        _plot_scales(df=df, name='simple', kind= 'scale')
        _plot_scales(df=df, name='simple', kind='ctr_dt')
# -----------------------------------
def _set_logs():
    LogStore.set_level('dmu:workflow:cache'   , 10)
    LogStore.set_level('rx_data:rdf_getter'   , 30)
    LogStore.set_level('rx_data:path_splitter', 30)
    LogStore.set_level('dmu:workflow:cache'   , 30)
    LogStore.set_level('rx_selection:truth_matching', 30)
    LogStore.set_level('rx_selection:selection'     , 30)
# -----------------------------------
