'''
Module used to test bias corrections
'''

import os
os.environ["CUDA_VISIBLE_DEVICES"] = '1'

import copy
from importlib.resources import files
from pathlib             import Path

import yaml
import numpy
import mplhep
import pytest
import matplotlib.pyplot as plt

from omegaconf                   import OmegaConf
from distributed                 import Client
from ROOT                        import RDF # type: ignore
from dmu.logging.log_store       import LogStore
from dmu.plotting.plotter_1d     import Plotter1D as Plotter
from rx_common                   import info
from rx_selection                import selection as sel
from rx_data                     import utilities as ut
from rx_data.rdf_getter          import RDFGetter
from rx_data.mass_bias_corrector import MassBiasCorrector

log=LogStore.add_logger('rx_data:test_mass_bias_corrector')

_SAMPLES_MM = [
    ('Bu_JpsiK_mm_eq_DPC'  , 'Hlt2RD_BuToKpMuMu_MVA'   ),
    ('Bd_JpsiKst_mm_eq_DPC', 'Hlt2RD_B0ToKpPimMuMu_MVA'),
    #----------------
    ('DATA_24_MagUp_24c2'  , 'Hlt2RD_BuToKpMuMu_MVA'   ),
    ('DATA_24_MagUp_24c2'  , 'Hlt2RD_B0ToKpPimMuMu_MVA'),
]

_SAMPLES_EE = [
    ('Bu_JpsiK_ee_eq_DPC'  , 'Hlt2RD_BuToKpEE_MVA'     ),
    ('Bd_JpsiKst_ee_eq_DPC', 'Hlt2RD_B0ToKpPimEE_MVA'  ),
    #----------------
    ('DATA_24_MagUp_24c2'  , 'Hlt2RD_BuToKpEE_MVA'     ),
    ('DATA_24_MagUp_24c2'  , 'Hlt2RD_B0ToKpPimEE_MVA'  )
]

_SAMPLES = _SAMPLES_MM + _SAMPLES_EE
#-----------------------------------------
class Data:
    '''
    Data class
    '''
    user       = os.environ['USER']
    plt_dir    = Path(f'/tmp/{user}/tests/rx_data/mass_bias_corrector')
    nthreads   = 13
    nentries   = -1
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before tests
    '''
    LogStore.set_level('rx_data:mass_bias_corrector'     , 10)
    LogStore.set_level('rx_data:electron_bias_corrector' , 20)
    LogStore.set_level('rx_data:test_mass_bias_corrector', 10)
    LogStore.set_level('rx_data:rdf_getter'              , 10)

    Data.plt_dir.mkdir(parents=True, exist_ok=True)

    plt.style.use(mplhep.style.LHCb2)
#-----------------------------------------
def _load_conf() -> dict:
    cfg_path = files('rx_data_data').joinpath('tests/mass_bias_corrector/mass_overlay.yaml')
    cfg_path = str(cfg_path)
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
#-----------------------------------------
def _clean_rdf(rdf : RDF.RNode, name : str) -> RDF.RNode:
    if name == 'Original':
        rdf = rdf.Define('Jpsi_M_smr', 'Jpsi_M')

    rdf = rdf.Filter('Jpsi_M > 0', 'pos_jmass')
    rdf = rdf.Filter('B_M    > 0', 'pos_bmass')

    rep = rdf.Report()
    rep.Print()

    return rdf
#-----------------------------------------
def _compare_masses(
    d_rdf      : dict[str, RDF.RNode], 
    test_name  : str, 
    correction : str,
    skip_jpsi  : bool=False) -> None:
    '''
    d_rdf     : Dictionary with corrected and original dataframes
    test_name : Used for output path
    correction: Correction name, used for title
    skip_jpsi : If false, will plot Jpsi_M distributin
    '''
    d_rdf = { name : _clean_rdf(rdf, name) for name, rdf in d_rdf.items() }

    cfg = _load_conf()
    cfg = copy.deepcopy(cfg)
    plt_dir = f'{Data.plt_dir}/{test_name}'

    cfg['saving'] = {'plt_dir' : plt_dir}

    cfg['plots']['B_M'   ]['title'] = correction

    if not skip_jpsi:
        cfg['plots']['Jpsi_M']['title'] = correction
    else:
        del cfg['plots']['Jpsi_M']

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
#-----------------------------------------
def _check_input_columns(rdf : RDF.RNode) -> None:
    l_colname = [ name.c_str() for name in rdf.GetColumnNames() ]

    l_track_brem = [ name for name in l_colname if name.endswith('BREMTRACKBASEDENERGY') ]

    if len(l_track_brem) == 0:
        for colname in l_colname:
            log.warning(colname)
        raise ValueError('No BREMTRACKBASEDENERGY found')

    log.info(f'Found: {l_track_brem}')
#-----------------------------------------
def _check_output_columns(rdf : RDF.RNode) -> None:
    l_colname = [ name.c_str() for name in rdf.GetColumnNames() ]
    ncol = len(l_colname)
    if ncol != 20:
        for colname in l_colname:
            log.info(f'   {colname}')

        raise ValueError(f'Expected 14 columns, got {ncol}')

    for colname in l_colname:
        log.debug(f'   {colname}')
#-----------------------------------------
def _get_rdf(
    trigger  : str,
    q2bin    : str       = 'jpsi',
    nbrem    : None|int  = None,
    is_inner : None|bool = None,
    npvs     : None|int  = None,
    bdt      : None|str  = None,
    sample   : None|str  = None,
    is_mc    : bool      = False) -> RDF.RNode:
    '''
    Return ROOT dataframe needed for test
    '''
    project = info.project_from_trigger(trigger=trigger, lower_case=True)
    if isinstance(sample, str):
        pass
    elif not is_mc:
        sample = 'DATA_24_*'
    elif   is_mc and project == 'rkst':
        sample = 'Bd_Kstee_eq_btosllball05_DPC'
    elif   is_mc and project == 'rk':
        sample = 'Bu_Kee_eq_btosllball05_DPC'
    else:
        raise ValueError(f'Invalid project: {project}')

    with RDFGetter.exclude_friends(names=['brem_track_2']):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    d_sel = sel.selection(
        trigger = trigger, 
        q2bin   = q2bin, 
        smeared = False,
        process = sample)

    # Do not use part reco removal for MC
    d_sel['mass'] = '(1)' if is_mc       else 'B_const_mass_M > 5160'
    d_sel['bdt']  = '(1)' if bdt is None else bdt

    # We run over 1000 entries to speed up tests
    # Those are from pre-UT data, which the block
    # requirement removes. Need to drop that requirement
    if not is_mc:
        del d_sel['block']

    for name, cut in d_sel.items():
        log.debug(f'{name:<20}{cut}')
        rdf = rdf.Filter(cut, name)

    if nbrem    is not None:
        brem_cut = f'nbrem == {nbrem}' if nbrem in [0,1] else f'nbrem >= {nbrem}'
        rdf = rdf.Filter(brem_cut)

    if is_inner is not None and     is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA == 2 && L2_BREMHYPOAREA == 2')

    if is_inner is not None and not is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA != 2 && L2_BREMHYPOAREA != 2')

    if npvs     is not None:
        rdf = rdf.Filter(f'nPVs == {npvs}')

    _check_input_columns(rdf)

    nentries = rdf.Count().GetValue()
    if nentries == 0:
        rep = rdf.Report()
        rep.Print()
        raise ValueError('Empty dataframe found')
    else:
        log.info(f'Found {nentries} entries')

    return rdf
# ----------------------
def _check_size(rdf_org : RDF.RNode, rdf_cor : RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf_org: Input dataframe, needed to be corrected
    rdf_cor: Dataframe with corrected data
    '''
    in_size = rdf_org.Count().GetValue()
    ot_size = rdf_cor.Count().GetValue()

    assert in_size == ot_size

    log.info(f'Sizes agree at: {in_size}')
#-----------------------------------------
@pytest.mark.parametrize('kind'   , ['brem_track_2'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_simple(kind : str, trigger : str):
    '''
    Simplest test
    '''
    rdf_org = _get_rdf(trigger=trigger)
    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df        = df_org, 
        is_mc     = is_mc,
        trigger   = trigger,
        nthreads  = 6, 
        ecorr_kind= kind)

    df_cor = cor.get_df()
    rdf_cor= RDF.FromPandas(df_cor)

    _check_size(rdf_org=rdf_org, rdf_cor=rdf_cor)
    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'simple/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
@pytest.mark.parametrize('sample', [
    'DATA_24_MagDown_24c2',
    'DATA_24_MagDown_24c3',
    'DATA_24_MagDown_24c4',
    'DATA_24_MagUp_24c2' ,
    'DATA_24_MagUp_24c3' ,
    'DATA_24_MagUp_24c4' ])
def test_medium_input(trigger : str, sample : str):
    '''
    Medium input
    '''
    kind    = 'brem_track_2'
    nproc   = 10

    with RDFGetter.max_entries(100_000):
        rdf_org = _get_rdf(sample=sample, trigger=trigger)

    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    with Client(n_workers=nproc):
        cor   = MassBiasCorrector(
            df        = df_org, 
            is_mc     = is_mc,
            trigger   = trigger,
            nthreads  = nproc, 
            ecorr_kind= kind)

        df_cor  = cor.get_df()

    rdf_cor = RDF.FromPandas(df_cor)

    _check_size(rdf_org=rdf_org, rdf_cor=rdf_cor)
    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'medium_{sample}/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('nbrem'  , [0, 1, 2])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_nbrem(nbrem : int, kind : str, trigger : str):
    '''
    Test splitting by brem
    '''
    rdf_org = _get_rdf(nbrem=nbrem, trigger=trigger)
    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df        = df_org, 
        is_mc     = is_mc,
        trigger   = trigger,
        nthreads  = Data.nthreads, 
        ecorr_kind= kind)

    df_cor  = cor.get_df()
    rdf_cor = RDF.FromPandas(df_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _check_size(rdf_org=rdf_org, rdf_cor=rdf_cor)
    _compare_masses(d_rdf, f'nbrem_{nbrem:03}/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('is_inner', [True, False])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_isinner(is_inner : bool, kind : str, trigger : str):
    '''
    Test splitting detector region
    '''
    rdf_org = _get_rdf(is_inner = is_inner, trigger = trigger)
    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df        = df_org, 
        is_mc     = is_mc,
        trigger   = trigger,
        nthreads  = Data.nthreads, 
        ecorr_kind= kind)

    df_cor  = cor.get_df()
    rdf_cor = RDF.FromPandas(df_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'is_inner_{is_inner}/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('nbrem', [1, 2])
@pytest.mark.parametrize('npvs' , [1, 3, 5, 7])
def test_nbrem_npvs(
    nbrem  : int, 
    npvs   : int, 
    trigger: str,
    kind   : str):
    '''
    Split by brem and nPVs
    '''
    with RDFGetter.max_entries(value=30_000):
        rdf_org = _get_rdf(nbrem=nbrem, npvs=npvs, trigger=trigger)

    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df      =df_org, 
        trigger =trigger,
        is_mc   =is_mc,
        nthreads=Data.nthreads, 
        ecorr_kind=kind)

    df_cor  = cor.get_df()

    rdf_cor = RDF.FromPandas(df_cor)
    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'brem_npvs_{nbrem}_{npvs}/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_suffix(kind : str, trigger : str):
    '''
    Tests that output dataframe has columns with suffix added
    '''
    rdf_org = _get_rdf(trigger=trigger)
    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df      =df_org, 
        trigger =trigger,
        is_mc   =is_mc,
        nthreads=Data.nthreads, 
        ecorr_kind=kind)
    df_cor = cor.get_df(suffix=kind)
    rdf_cor= RDF.FromPandas(df_cor)

    _check_output_columns(rdf_cor)
#-----------------------------------------
@pytest.mark.parametrize('nbrem', [0, 1])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
@pytest.mark.parametrize('brem_energy_threshold', [100, 200, 300, 400, 600, 800, 1000, 1500, 2000, 4000])
def test_brem_threshold(nbrem : int, brem_energy_threshold: float, trigger : str):
    '''
    Test splitting by brem
    '''
    with RDFGetter.max_entries(value=50_000):
        rdf_org = _get_rdf(nbrem=nbrem, trigger=trigger)

    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df        =df_org, 
        trigger   =trigger,
        is_mc     =is_mc,
        nthreads  =Data.nthreads, 
        brem_energy_threshold=brem_energy_threshold)

    df_cor = cor.get_df()

    rdf_cor= RDF.FromPandas(df_cor)

    d_rdf  = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'brem_{nbrem:03}/{trigger}/energy_{brem_energy_threshold:03}', f'$E_{{\\gamma}}>{brem_energy_threshold}$ MeV')
#-----------------------------------------
@pytest.mark.parametrize('sample, trigger', _SAMPLES) 
def test_add_smearing(sample : str, trigger : str):
    '''
    Checks that smearing of q2 was added on top of correction for electron samples
    '''
    is_mc   = sample.startswith('DATA_')

    rdf_org = _get_rdf(is_mc=is_mc, trigger=trigger)
    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df        =df_org, 
        trigger   =trigger,
        is_mc     =is_mc,
        nthreads  =10, 
        ecorr_kind=kind)

    df_cor = cor.get_df()
    rdf_cor= RDF.FromPandas(df_cor)
    _check_output_columns(rdf_cor)

    # Only electron has to be brem corrected
    _check_corrected(must_be_corrected=info.is_ee(trigger), rdf=rdf_cor, name='Jpsi_M', kind = 'brem_track_2')
    _check_corrected(must_be_corrected=info.is_ee(trigger), rdf=rdf_cor, name=   'B_M', kind = 'brem_track_2')

    # Only MC has to be smeared
    _check_corrected(must_be_corrected=is_mc              , rdf=rdf_cor, name='Jpsi_M', kind = 'smr')
    _check_corrected(must_be_corrected=is_mc              , rdf=rdf_cor, name=   'B_M', kind = 'smr')

    rdf_smr = rdf_cor.Redefine('Jpsi_M', 'Jpsi_M_smr')
    rdf_smr = rdf_smr.Redefine(   'B_M',    'B_M_smr')

    sample  = 'mc' if is_mc else 'data'
    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor, 'Smeared' : rdf_smr}
    _compare_masses(d_rdf = d_rdf, test_name = f'add_smearing_{sample}', correction = sample)
# ----------------------
def _check_corrected(
    must_be_corrected: bool, 
    name             : str,
    kind             : str,
    rdf              : RDF.RNode) -> None:
    '''
    Parameters
    -------------
    must_be_smeared: If true, will raise if unsmeared and smeared are equal.
    name           : Variable to be checked
    kind           : Type of correction, suffix for corrected branch
    rdf            : DataFrame after correction
    '''
    arr_val_org = rdf.AsNumpy([         name])[         name]
    arr_val_smr = rdf.AsNumpy([f'{name}_{kind}'])[f'{name}_{kind}']

    if not must_be_corrected:
        assert numpy.isclose(arr_val_org, arr_val_smr, rtol=1e-5)
    else:
        assert not numpy.isclose(arr_val_org, arr_val_smr, rtol=1)
#-----------------------------------------
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
@pytest.mark.parametrize('is_mc'  , [True, False])
def test_dask(trigger : str, is_mc : bool):
    '''
    Test processing in parallel with Dask client
    '''
    kind    = 'brem_track_2'
    nproc   = 4 

    with RDFGetter.max_entries(10_000):
        rdf_org = _get_rdf(is_mc=is_mc, trigger=trigger)

    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    with Client(n_workers=nproc, threads_per_worker=1):
        cor   = MassBiasCorrector(
            df        = df_org, 
            is_mc     = is_mc,
            trigger   = trigger,
            nthreads  = nproc, 
            ecorr_kind= kind)

        df_cor  = cor.get_df()

    rdf_cor = RDF.FromPandas(df_cor)

    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'dask_{is_mc}/{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('trigger', [
    'Hlt2RD_BuToKpEE_MVA', 
    'Hlt2RD_B0ToKpPimEE_MVA'])
def test_signal(trigger : str):
    '''
    Tests for signal
    '''
    kind = 'brem_track_2'
    with RDFGetter.max_entries(value=100_000):
        rdf_org = _get_rdf(trigger=trigger, is_mc=True, q2bin='low')

    df_org  = ut.df_from_rdf(rdf=rdf_org, drop_nans=False)
    is_mc   = ut.rdf_is_mc(rdf=rdf_org)

    cor     = MassBiasCorrector(
        df             = df_org, 
        is_mc          = is_mc,
        trigger        = trigger,
        nthreads       = 10, 
        skip_correction= False,
        ecorr_kind     = kind)

    df_cor = cor.get_df()
    rdf_cor= RDF.FromPandas(df_cor)

    _check_size(rdf_org=rdf_org, rdf_cor=rdf_cor)
    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'signal/{trigger}', kind, skip_jpsi=True)
#-----------------------------------------
