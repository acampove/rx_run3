'''
Module used to test bias corrections
'''

import os
from importlib.resources import files

import pytest
import yaml

from ROOT import RDataFrame
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
    plt_dir = '/tmp/tests/rx_data/mass_bias_corrector'
#-----------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('rx_data:mass_bias_corrector', 10)

    os.makedirs(Data.plt_dir, exist_ok=True)
#-----------------------------------------
def _load_conf() -> dict:
    cfg_path = files('rx_data_data').joinpath('tests/mass_bias_corrector/mass_overlay.yaml')
    with open(cfg_path, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
#-----------------------------------------
def _compare_masses(d_rdf : dict[str,RDataFrame], name : str, title : str) -> None:
    cfg = _load_conf()
    cfg['plots']['B_M']['name' ] = name
    cfg['plots']['B_M']['title'] = title

    ptr=Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
#-----------------------------------------
def _get_rdf(polarity : str, period : str, is_in : int) -> RDataFrame:
    RDFGetter.samples = {
        'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
        'mva'  : '/home/acampove/external_ssd/Data/samples/mva.yaml',
        'hop'  : '/home/acampove/external_ssd/Data/samples/hop.yaml',
        }

    gtr = RDFGetter(sample=f'DATA_24_{polarity}_{period}', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()

    if is_in == 0:
        rdf = rdf.Filter('TMath::Abs(L1_STATEAT_Ecal_positionX) > 800 && TMath::Abs(L1_STATEAT_Ecal_positionY) > 400 && TMath::Abs(L2_STATEAT_Ecal_positionX) > 800 && TMath::Abs(L2_STATEAT_Ecal_positionY) > 400', 'out')

    if is_in == 1:
        rdf = rdf.Filter('TMath::Abs(L1_STATEAT_Ecal_positionX) < 800 && TMath::Abs(L1_STATEAT_Ecal_positionY) < 400 && TMath::Abs(L2_STATEAT_Ecal_positionX) < 800 && TMath::Abs(L2_STATEAT_Ecal_positionY) < 400', 'in')

    rdf = rdf.Filter('(Jpsi_M * Jpsi_M >  6000000) && (Jpsi_M * Jpsi_M < 12960000)')
    rdf = rdf.Filter('mva.mva_cmb > 0.5 && mva.mva_prc > 0.5')
    rdf = rdf.Filter('B_const_mass_M > 5160')
    rdf = rdf.Filter('hop.hop_mass > 4500')
    rdf = rdf.Range(10_000)

    return rdf
#-----------------------------------------
@pytest.mark.parametrize('is_in'   , [0, 1, 2])
@pytest.mark.parametrize('polarity', ['MagUp', 'MagDown'])
@pytest.mark.parametrize('period'  , ['24c1', '24c2', '24c3', '24c4'])
def test_correction(polarity : str, period : str, is_in : int):
    '''
    Tests code correcting lepton kinematics
    '''
    rdf_org = _get_rdf(polarity, period, is_in)

    cor     = MassBiasCorrector(rdf=rdf_org)
    rdf_cor = cor.get_rdf()

    d_rdf   = {'Original' : rdf_org, 'Corrected' : rdf_cor}

    title = f'{polarity}; {period}; Inner={is_in}'
    _compare_masses(d_rdf, f'correction_{polarity}_{period}_{is_in:03}', title)
#-----------------------------------------
