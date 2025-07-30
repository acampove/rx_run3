'''
Module with functions meant to test SampleSplitter class
'''
import os

import pytest
import mplhep
import matplotlib.pyplot as plt
import pandas as pnd
from dmu.generic                   import utilities   as gut
from dmu.logging.log_store         import LogStore
from rx_data.rdf_getter            import RDFGetter
from rx_misid.data_sample_splitter import SampleSplitter

log=LogStore.add_logger('rx_misid:test_data_splitter')
# -------------------------------------------------------
class Data:
    '''
    Data class
    '''
    plt.style.use(mplhep.style.LHCb2)
    user    = os.environ['USER']
    out_dir = f'/tmp/{user}/tests/rx_misid/sample_splitter'
# -------------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_misid:test_splitter', 10)
    LogStore.set_level('rx_misid:splitter'     , 10)
    LogStore.set_level('rx_data:rdf_getter'    , 30)
    LogStore.set_level('rx_data:path_splitter' , 30)

    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------------------------------
def _get_rdf(sample : str, trigger : str, project : str):
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis=project)
    rdf = gtr.get_rdf()
    uid = gtr.get_uid()
    rdf.uid = uid

    return rdf
# -------------------------------------------------------
def _check_stats(df : pnd.DataFrame):
    fail = False
    log.info(40 * '-')
    log.info(f'{"Kind":<20}{"Entries":<20}')
    log.info(40 * '-')
    for kind, df_kind in df.groupby('kind'):
        if len(df_kind) == 0:
            log.warning(f'Empty sample: {kind}')
            fail=True
            continue

        nentries = len(df_kind)

        log.info(f'{kind:<20}{nentries:<20}')
    log.info(40 * '-')

    assert not fail
# -------------------------------------------------------
def _plot_pide(df : pnd.DataFrame, sample : str) -> None:
    '''
    Parameters
    ---------------
    df    : Pandas dataframe with tagged candidates
    sample: E.g. DATA_24...
    '''
    for kind, df_kind in df.groupby('kind'):
        for hadron, df_hadr in df_kind.groupby('hadron'):
            ax = None
            ax = df_hadr.plot.scatter(x='L1_PID_E', y='L1_PROBNN_E', color='blue', s=1, label='$e_{SS}$', ax=ax)
            ax = df_hadr.plot.scatter(x='L2_PID_E', y='L2_PROBNN_E', color='red' , s=1, label='$e_{OS}$', ax=ax)

            plot_path = f'{Data.out_dir}/{sample}_{hadron}_{kind}.png'

            ax.set_xlabel(r'$\Delta LL (e)$')
            ax.set_ylabel('ProbNN(e)')

            bname = '$B^+$'
            plt.title(f'{hadron}; {bname}; {kind}')
            plt.savefig(plot_path)
            plt.close()
# -------------------------------------------------------
def test_data():
    '''
    Tests getting split dataset 
    '''
    sample= 'DATA_24_MagUp_24c2'
    log.info('')

    rdf   = _get_rdf(
        sample = sample,
        trigger= 'Hlt2RD_BuToKpEE_MVA_ext',
        project= 'rx')

    cfg   = gut.load_conf(package='rx_misid_data', fpath='splitting.yaml') 
    spl   = SampleSplitter(rdf = rdf, cfg = cfg)
    df    = spl.get_sample()

    log.info('Dataframe found, checking')
    _check_stats(df=df)
    _plot_pide(df=df, sample=sample)
# -------------------------------------------------------
