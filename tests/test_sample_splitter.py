'''
Module with functions meant to test SampleSplitter class
'''
import os

import numpy
import pytest
import mplhep
import matplotlib.pyplot as plt
import pandas as pnd
from ROOT                     import RDataFrame # type: ignore
from dmu.generic              import utilities   as gut
from dmu.logging.log_store    import LogStore
from rx_selection             import selection   as sel
from rx_data.rdf_getter       import RDFGetter
from rx_misid.sample_splitter import SampleSplitter

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
def initialize():
    '''
    This runs before any test
    '''
    LogStore.set_level('rx_misid:test_splitter', 10)
    LogStore.set_level('rx_misid:splitter'     , 10)
    LogStore.set_level('rx_data:rdf_getter'    , 30)
    LogStore.set_level('rx_data:path_splitter' , 30)

    os.makedirs(Data.out_dir, exist_ok=True)
# -------------------------------------------------------
def _get_rdf(sample : str, trigger : str, project : str):
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis=project)
    rdf = gtr.get_rdf(per_file=False)
    uid = gtr.get_uid()
    with sel.custom_selection(d_sel = {'pid_l' : '(1)'}):
        rdf = sel.apply_full_selection(
            rdf    = rdf,
            uid    = uid,
            q2bin  = 'central',
            process= sample,
            trigger= trigger)

    return rdf
# -------------------------------------------------------
def _plot_data_pide(df : pnd.DataFrame, sample : str) -> None:
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
def _plot_simulation_pide(df : pnd.DataFrame, sample : str) -> None:
    '''
    Parameters
    ---------------
    df    : Pandas dataframe with tagged candidates
    sample: E.g. Bu_Kee_eq_btosllball05_DPC...
    '''
    for hadron, df_hadr in df.groupby('hadron'):
        ax = None
        ax = df_hadr.plot.scatter(x='L1_PID_E', y='L1_PROBNN_E', color='blue', s=1, label='$e_{SS}$', ax=ax)
        ax = df_hadr.plot.scatter(x='L2_PID_E', y='L2_PROBNN_E', color='red' , s=1, label='$e_{OS}$', ax=ax)

        plot_path = f'{Data.out_dir}/{sample}_{hadron}.png'

        ax.set_xlabel(r'$\Delta LL (e)$')
        ax.set_ylabel('ProbNN(e)')

        bname = '$B^+$'
        plt.title(f'{hadron}; {bname}')
        plt.savefig(plot_path)
        plt.close()
# -------------------------------------------------------
def _check_dt_stats(rdf : RDataFrame, df : pnd.DataFrame):
    '''
    Parameters
    -------------
    rdf : ROOT dataframe used as input when doing data tests
    df  : Pandas dataframe in the output
    '''
    ninput = rdf.Count().GetValue()
    noutput= len(df)

    # The input contains the signal region
    # The output contains FF, PF, FP only
    assert noutput < ninput

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
# ----------------------
def _check_mc_stats(rdf : RDataFrame, df : pnd.DataFrame) -> None:
    '''
    Parameters
    -------------
    rdf : ROOT dataframe used as input when doing MC tests
    df  : Pandas dataframe in the output
    '''
    ninput = rdf.Count().GetValue()
    noutput= len(df)

    assert ninput == noutput

    arr_block_rdf = rdf.AsNumpy(['block'])['block']
    arr_block_pnd = df['block'].to_numpy()

    # Use sequence of blocks to check that
    # There is no shufflign of entries for MC
    assert numpy.array_equal(arr_block_rdf, arr_block_pnd)
# ----------------------
def _check_columns(df : pnd.DataFrame, is_mc : bool) -> None:
    '''
    Parameters
    -------------
    df: Pandas dataframe produced by SampleSplitter
    '''
    kinds = df['kind'].unique() 

    if not is_mc:
        assert set(kinds) <= {'FailFail', 'FailPass', 'PassFail'}
    else:
        assert set(kinds) <= {'N/A'}

    s_expected = {
        'B_Mass',
        'B_Mass_smr',
        'block',
        'L1_HASBREM',
        'L2_HASBREM',
        'L1_PID_E',
        'L2_PID_E',
        'L1_PROBNN_E',
        'L2_PROBNN_E',
        'L1_TRACK_PT',
        'L1_TRACK_ETA',
        'L2_TRACK_PT',
        'L2_TRACK_ETA',
        'kind',
        'weight',
        'hadron'}

    assert set(df.columns) == s_expected
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
    _check_dt_stats(df=df, rdf=rdf)
    _plot_data_pide(df=df, sample=sample)
    _check_columns(df=df)
# -------------------------------------------------------
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC',
    'Bu_KplKplKmn_eq_sqDalitz_DPC'])
def test_simulation(sample : str):
    '''
    Tests getting split dataset
    '''
    log.info('')

    rdf   = _get_rdf(
        sample = sample,
        trigger= 'Hlt2RD_BuToKpEE_MVA_noPID',
        project= 'nopid')

    cfg   = gut.load_conf(package='rx_misid_data', fpath='splitting.yaml')
    spl   = SampleSplitter(rdf = rdf, cfg = cfg)
    df    = spl.get_sample()

    log.info('Dataframe found, checking')
    _plot_simulation_pide(df=df, sample=sample)
    _check_mc_stats(rdf=rdf, df=df)
    _check_columns(df=df)
# -------------------------------------------------------
