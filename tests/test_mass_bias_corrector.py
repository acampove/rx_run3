'''
Module used to test bias corrections
'''

import os
import copy
from importlib.resources import files
from pathlib             import Path

import mplhep
import pytest
import yaml
import matplotlib.pyplot as plt

from ROOT                        import RDataFrame # type: ignore
from dmu.logging.log_store       import LogStore
from dmu.plotting.plotter_1d     import Plotter1D as Plotter
from rx_common                   import info
from rx_selection                import selection as sel
from rx_data.rdf_getter          import RDFGetter
from rx_data.mass_bias_corrector import MassBiasCorrector

log=LogStore.add_logger('rx_data:test_mass_bias_corrector')
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
def _clean_rdf(rdf : RDataFrame, name : str) -> RDataFrame:
    if name == 'Original':
        rdf = rdf.Define('Jpsi_M_smr', 'Jpsi_M')

    rdf = rdf.Filter('Jpsi_M > 0', 'pos_jmass')
    rdf = rdf.Filter('B_M    > 0', 'pos_bmass')

    rep = rdf.Report()
    rep.Print()

    return rdf
#-----------------------------------------
def _compare_masses(d_rdf : dict[str,RDataFrame], test_name : str, correction : str) -> None:
    d_rdf = { name : _clean_rdf(rdf, name) for name, rdf in d_rdf.items() }

    cfg = _load_conf()
    cfg = copy.deepcopy(cfg)
    plt_dir = f'{Data.plt_dir}/{test_name}'

    cfg['saving'] = {'plt_dir' : plt_dir}

    cfg['plots']['B_M'   ]['title'] = correction
    cfg['plots']['Jpsi_M']['title'] = correction

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
#-----------------------------------------
def _check_input_columns(rdf : RDataFrame) -> None:
    l_colname = [ name.c_str() for name in rdf.GetColumnNames() ]

    l_track_brem = [ name for name in l_colname if name.endswith('BREMTRACKBASEDENERGY') ]

    if len(l_track_brem) == 0:
        for colname in l_colname:
            log.warning(colname)
        raise ValueError('No BREMTRACKBASEDENERGY found')

    log.info(f'Found: {l_track_brem}')
#-----------------------------------------
def _check_output_columns(rdf : RDataFrame) -> None:
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
    nbrem    : None|int  = None,
    is_inner : None|bool = None,
    npvs     : None|int  = None,
    bdt      : None|str  = None,
    sample   : None|str  = None,
    is_mc    : bool      = False) -> RDataFrame:
    '''
    Return ROOT dataframe needed for test
    '''
    project = info.project_from_trigger(trigger=trigger, lower_case=True)

    if     is_mc and sample is None and project == 'rkstar':
        sample = 'Bd_Kstee_eq_btosllball05_DPC'

    if     is_mc and sample is None and project == 'rk':
        sample = 'Bu_Kee_eq_btosllball05_DPC'

    if not is_mc and sample is None:
        sample = 'DATA_24_*'

    if sample is None:
        raise ValueError('Sample not defined')

    with RDFGetter.exclude_friends(names=['brem_track_2']):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    d_sel = sel.selection(trigger=trigger, q2bin='jpsi', process=sample)
    d_sel['mass'] = 'B_const_mass_M > 5160'
    d_sel['bdt']  = '(1)'

    if bdt   is not None:
        d_sel['bdt' ] = bdt

    # We run over 1000 entries to speed up tests
    # Those are from pre-UT data, which the block
    # requirement removes. Need to drop that requirement
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
#-----------------------------------------
@pytest.mark.parametrize('kind'   , ['brem_track_2'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_B0ToKpPimEE_MVA'])
def test_simple(kind : str, trigger : str):
    '''
    Simplest test
    '''
    rdf_org = _get_rdf(trigger=trigger)
    cor     = MassBiasCorrector(
        rdf       = rdf_org, 
        trigger   = trigger,
        nthreads  = 6, 
        ecorr_kind= kind)

    rdf_cor = cor.get_rdf()

    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'simple_{trigger}', kind)
#-----------------------------------------
@pytest.mark.parametrize('sample', [
    'DATA_24_MagDown_24c1',
    'DATA_24_MagDown_24c2',
    'DATA_24_MagDown_24c3',
    'DATA_24_MagDown_24c4',
    'DATA_24_MagUp_24c1' ,
    'DATA_24_MagUp_24c2' ,
    'DATA_24_MagUp_24c3' ,
    'DATA_24_MagUp_24c4' ])
def test_medium_input(sample : str):
    '''
    Medium input
    '''
    kind    = 'brem_track_2'

    with RDFGetter.max_entries(100_000):
        rdf_org = _get_rdf(sample=sample)

    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=6, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'medium_{sample}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('nbrem'  , [0, 1, 2])
def test_nbrem(nbrem : int, kind : str):
    '''
    Test splitting by brem
    '''
    rdf_org = _get_rdf(nbrem=nbrem)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'nbrem_{nbrem:03}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('is_inner', [True, False])
def test_isinner(is_inner : bool, kind : str):
    '''
    Test splitting detector region
    '''
    rdf_org = _get_rdf(is_inner = is_inner)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'is_inner_{is_inner}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
@pytest.mark.parametrize('nbrem', [0, 1, 2])
@pytest.mark.parametrize('npvs' , [1, 2, 3, 4, 5, 6, 7])
def test_nbrem_npvs(nbrem : int, npvs : int, kind : str):
    '''
    Split by brem and nPVs
    '''
    rdf_org = _get_rdf(nbrem=nbrem, npvs=npvs)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'brem_npvs_{nbrem}_{npvs}', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_2'])
def test_suffix(kind : str):
    '''
    Tests that output dataframe has columns with suffix added
    '''
    rdf_org = _get_rdf()
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf(suffix=kind)

    _check_output_columns(rdf_cor)
#-----------------------------------------
@pytest.mark.parametrize('nbrem', [0, 1])
@pytest.mark.parametrize('brem_energy_threshold', [100, 200, 300, 400, 600, 800, 1000, 1500, 2000, 4000])
def test_brem_threshold(nbrem : int, brem_energy_threshold: float):
    '''
    Test splitting by brem
    '''
    rdf_org = _get_rdf(nbrem=nbrem)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind='brem_track_2', brem_energy_threshold=brem_energy_threshold)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    _compare_masses(d_rdf, f'brem_{nbrem:03}/energy_{brem_energy_threshold:03}', f'$E_{{\\gamma}}>{brem_energy_threshold}$ MeV')
#-----------------------------------------
@pytest.mark.parametrize('kind' , ['brem_track_2'])
@pytest.mark.parametrize('is_mc', [True, False])
def test_add_smearing(kind : str, is_mc : bool):
    '''
    Checks that smearing of q2 was added on top of correction
    '''
    rdf_org = _get_rdf(is_mc=is_mc, bdt='(1)')
    rdf_org = rdf_org.Range(50_000)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=10, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()
    _check_output_columns(rdf_cor)

    rdf_smr = rdf_cor.Redefine('Jpsi_M', 'Jpsi_M_smr')
    rdf_smr = rdf_smr.Redefine(   'B_M',    'B_M_smr')

    sample  = 'mc' if is_mc else 'data'
    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor, 'Smeared' : rdf_smr}
    _compare_masses(d_rdf, f'add_smearing_{sample}', kind)
#-----------------------------------------
