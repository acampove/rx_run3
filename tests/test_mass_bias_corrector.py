'''
Module used to test bias corrections
'''

import os

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT import RDataFrame
from dmu.logging.log_store       import LogStore
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
def _compare_masses(rdf : RDataFrame, name : str) -> None:
    d_mass = rdf.AsNumpy(['B_M', 'B_M_corr'])
    df     = pnd.DataFrame(d_mass)
    df.plot.hist(bins=40, histtype='step', range=[4000, 6000])
    plt.savefig(f'{Data.plt_dir}/{name}.png')
    plt.close()
#-----------------------------------------
def _get_rdf() -> RDataFrame:
    RDFGetter.samples = {
        'main' : '/home/acampove/external_ssd/Data/samples/main.yaml',
        'mva'  : '/home/acampove/external_ssd/Data/samples/mva.yaml',
        'hop'  : '/home/acampove/external_ssd/Data/samples/hop.yaml',
        }

    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf()
    rdf = rdf.Filter('(Jpsi_M * Jpsi_M >  6000000) && (Jpsi_M * Jpsi_M < 12960000)')
    rdf = rdf.Filter('mva.mva_cmb > 0.5 && mva.mva_prc > 0.5')
    rdf = rdf.Filter('B_const_mass_M > 5160')
    rdf = rdf.Filter('hop.hop_mass > 4500')
    rdf = rdf.Range(10_000)

    return rdf
#-----------------------------------------
def test_skip_correction():
    '''
    Tests code without actually correcting lepton kinematics
    '''
    rdf = _get_rdf()

    cor = MassBiasCorrector(rdf=rdf)
    rdf = cor.get_rdf()

    _compare_masses(rdf, 'skip_correction')

    rdf.Display(nRows=20).Print()
#-----------------------------------------
