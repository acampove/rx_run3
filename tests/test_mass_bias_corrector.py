'''
Module used to test bias corrections
'''

import os
import copy
from importlib.resources import files

import mplhep
import pytest
import yaml
import matplotlib.pyplot as plt

from ROOT                        import RDataFrame, EnableImplicitMT, DisableImplicitMT
from dmu.logging.log_store       import LogStore
from dmu.plotting.plotter_1d     import Plotter1D as Plotter
from rx_selection                import selection as sel
from rx_data.rdf_getter          import RDFGetter
from rx_data.mass_bias_corrector import MassBiasCorrector

log=LogStore.add_logger('rx_data:test_mass_bias_corrector')
#-----------------------------------------
class Data:
    '''
    Data class
    '''
    plt_dir    = '/tmp/tests/rx_data/mass_bias_corrector'
    nthreads   = 13
    nentries   = -1
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:mass_bias_corrector'     , 10)
    LogStore.set_level('rx_data:test_mass_bias_corrector', 10)

    os.makedirs(Data.plt_dir, exist_ok=True)
    plt.style.use(mplhep.style.LHCb2)

    if Data.nentries < 0:
        EnableImplicitMT(Data.nthreads)
#-----------------------------------------
def _load_conf() -> dict:
    cfg_path = files('rx_data_data').joinpath('tests/mass_bias_corrector/mass_overlay.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
#-----------------------------------------
def _clean_rdf(rdf : RDataFrame) -> RDataFrame:
    rdf = rdf.Filter('Jpsi_M > 0', 'pos_jmass')
    rdf = rdf.Filter('B_M    > 0', 'pos_bmass')

    rep = rdf.Report()
    rep.Print()

    return rdf
#-----------------------------------------
def _compare_masses(d_rdf : dict[str,RDataFrame], test_name : str, correction : str) -> None:
    d_rdf = { name : _clean_rdf(rdf) for name, rdf in d_rdf.items() }

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
    if ncol != 14:
        for colname in l_colname:
            log.info(f'   {colname}')

        raise ValueError(f'Expected 14 columns, got {ncol}')

    for colname in l_colname:
        log.debug(f'   {colname}')
#-----------------------------------------
def _get_rdf(nbrem : int = None, is_inner : bool = None, npvs : int = None) -> RDataFrame:
    RDFGetter.samples = {
        'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
        'mva'  : '/home/acampove/external_ssd/Data/samples/mva.yaml',
        'hop'  : '/home/acampove/external_ssd/Data/samples/hop.yaml',
        'casc' : '/home/acampove/external_ssd/Data/samples/cascade.yaml',
        'jmis' : '/home/acampove/external_ssd/Data/samples/jpsi_misid.yaml',
        }

    gtr = RDFGetter(sample='DATA_24_*', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Define('nbrem', 'int(L1_HASBREMADDED) + int(L2_HASBREMADDED)')

    d_sel = sel.selection(project='RK', analysis='EE', q2bin='jpsi', process='DATA')
    d_sel['mass'] = 'B_const_mass_M > 5160'
    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    if nbrem is not None:
        brem_cut = f'nbrem == {nbrem}' if nbrem in [0,1] else f'nbrem >= {nbrem}'
        rdf = rdf.Filter(brem_cut)

    if is_inner is not None and     is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA == 2 && L2_BREMHYPOAREA == 2')

    if is_inner is not None and not is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA != 2 && L2_BREMHYPOAREA != 2')

    if npvs is not None:
        rdf = rdf.Filter(f'nPVs == {npvs}')

    if Data.nentries > 0:
        rdf = rdf.Range(Data.nentries)

    _check_input_columns(rdf)

    return rdf
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_1', 'brem_track_2'])
def test_small_input(kind : str):
    '''
    Run over a few entries
    '''
    DisableImplicitMT()

    rdf_org = _get_rdf()
    rdf_org = rdf_org.Range(10_000)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=1, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, 'small_input', kind)

    EnableImplicitMT(Data.nthreads)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_1', 'brem_track_2'])
def test_full_dataset(kind : str):
    '''
    Run over all the data, no binning
    '''
    rdf_org = _get_rdf()
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    _check_output_columns(rdf_cor)

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, 'full_dataset', kind)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['brem_track_1', 'brem_track_2'])
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
@pytest.mark.parametrize('kind', ['brem_track_1', 'brem_track_2'])
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
@pytest.mark.parametrize('kind', ['brem_track_1', 'brem_track_2'])
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
