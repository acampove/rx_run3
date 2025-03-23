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
from rx_data.rdf_getter          import RDFGetter
from rx_data.mass_bias_corrector import MassBiasCorrector

log=LogStore.add_logger('rx_data:test_mass_bias_corrector')
#-----------------------------------------
class Data:
    '''
    Data class
    '''
    plt_dir    = '/tmp/tests/rx_data/mass_bias_corrector'
    nthreads   = 10
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:mass_bias_corrector', 10)

    os.makedirs(Data.plt_dir, exist_ok=True)
    plt.style.use(mplhep.style.LHCb2)
#-----------------------------------------
def _load_conf() -> dict:
    cfg_path = files('rx_data_data').joinpath('tests/mass_bias_corrector/mass_overlay.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
#-----------------------------------------
def _compare_masses(d_rdf : dict[str,RDataFrame], name : str, title : str) -> None:
    cfg = _load_conf()
    cfg = copy.deepcopy(cfg)

    cfg['saving'] = {'plt_dir' : Data.plt_dir}

    cfg['plots']['B_M']['name' ] = name
    cfg['plots']['B_M']['title'] = title

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
#-----------------------------------------
def _check_columns(rdf : RDataFrame) -> None:
    l_colname = [ name.c_str() for name in rdf.GetColumnNames() ]

    l_track_brem = [ name for name in l_colname if name.endswith('BREMTRACKBASEDENERGY') ]

    if len(l_track_brem) == 0:
        for colname in l_colname:
            log.warning(colname)
        raise ValueError('No BREMTRACKBASEDENERGY found')

    log.info(f'Found: {l_track_brem}')
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
    rdf = rdf.Filter('(Jpsi_M * Jpsi_M >  6000000) && (Jpsi_M * Jpsi_M < 12960000)')
    rdf = rdf.Filter('mva.mva_cmb > 0.5 && mva.mva_prc > 0.5')
    rdf = rdf.Filter('B_const_mass_M > 5160')
    rdf = rdf.Filter('hop.hop_mass > 4500')
    rdf = rdf.Filter('jmis.swp_jpsi_misid_mass_swp < 3050 || jmis.swp_jpsi_misid_mass_swp > 3150')
    rdf = rdf.Filter('casc.swp_cascade_mass_swp > 1900')

    if nbrem is not None:
        brem_cut = f'nbrem == {nbrem}' if nbrem in [0,1] else f'nbrem >= {nbrem}'
        rdf = rdf.Filter(brem_cut)

    if is_inner is not None and     is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA == 2 && L2_BREMHYPOAREA == 2')

    if is_inner is not None and not is_inner:
        rdf = rdf.Filter('L1_BREMHYPOAREA != 2 && L2_BREMHYPOAREA != 2')

    if npvs is not None:
        rdf = rdf.Filter(f'nPVs == {npvs}')

    _check_columns(rdf)

    rdf = rdf.Range(100)

    return rdf
#-----------------------------------------
@pytest.mark.parametrize('kind', ['ecalo_bias', 'brem_track_1', 'brem_track_2'])
def test_minimal(kind : str):
    '''
    Fastest/Simplest test
    '''
    rdf_org = _get_rdf()
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=1, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}
    _compare_masses(d_rdf, f'minimal_{kind}', f'minimal {kind}')
#-----------------------------------------
@pytest.mark.parametrize('kind', ['ecalo_bias', 'brem_track_1'])
@pytest.mark.parametrize('nbrem'  , [0, 1, 2])
def test_nbrem(nbrem : int, kind : str):
    '''
    Test splitting by brem
    '''
    rdf_org = _get_rdf(nbrem=nbrem)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    title   = f'brem={nbrem}'
    _compare_masses(d_rdf, f'nbrem_{nbrem:03}', title)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['ecalo_bias', 'brem_track_1'])
@pytest.mark.parametrize('is_inner', [True, False])
def test_isinner(is_inner : bool, kind : str):
    '''
    Test splitting detector region
    '''
    rdf_org = _get_rdf(is_inner = is_inner)
    cor     = MassBiasCorrector(rdf=rdf_org, nthreads=Data.nthreads, ecorr_kind=kind)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    title   = f'isInner={is_inner}'
    _compare_masses(d_rdf, f'is_inner_{is_inner}', title)
#-----------------------------------------
@pytest.mark.parametrize('kind', ['ecalo_bias', 'brem_track_1'])
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

    title   = f'nPVs={npvs}; nbrem={nbrem}'
    _compare_masses(d_rdf, f'brem_npvs_{nbrem}_{npvs}', title)
#-----------------------------------------
