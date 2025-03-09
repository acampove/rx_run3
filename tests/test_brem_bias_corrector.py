'''
Module with functions needed to test BremBiasCorrector class
'''
import os
import numpy
import pytest
import matplotlib.pyplot as plt

from vector                      import MomentumObject4D as v4d
from dmu.logging.log_store       import LogStore
from rx_data.brem_bias_corrector import BremBiasCorrector
from rx_data                     import calo_translator as ctran

log=LogStore.add_logger('rx_data:test_brem_bias_corrector')
# -----------------------------------------------
class Data:
    '''
    Data class
    '''
    plt_dir = '/tmp/tests/rx_data/bias_corrector'

    locations : list[list]
# -----------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    df = ctran.get_data()
    df = df.drop(columns=['n', 'z'])

    Data.locations = df.values.tolist()
# -----------------------------------------------
def _get_input(energy : float):
    br_1 = v4d(pt=5_000, eta=3.0, phi=1.0, mass=0.511)
    br_2 = v4d(px=br_1.px, py=br_1.py, pz=br_1.pz, e=energy)

    return br_2
# -----------------------------------------------
@pytest.mark.parametrize('energy', [2_000, 4_000, 6_000, 8_000, 10_000])
def test_scan(energy : float):
    '''
    Will scan the calorimeter and plot corrections
    '''
    brem   = _get_input(energy=energy)
    obj    = BremBiasCorrector()
    l_x    = []
    l_y    = []
    l_corr = []
    for area, x, y, row, col in Data.locations:
        brem_corr = obj.correct(brem=brem, row=row, col=col, area=area)
        energy_corr = brem_corr.e

        mu = energy / energy_corr
        if mu < 0.5 or mu > 3.0:
            log.warning(f'Found correction: {mu:.3f}')

        l_corr.append(mu)
        l_x.append(x)
        l_y.append(y)

    plt.contourf(l_x, l_y, l_corr, levels=50,vmin=0.9, vmax=2.0)
    plt.colorbar(label='Correction')

    os.makedirs(Data.plt_dir, exist_ok=True)

    plt.xlabel("X values")
    plt.ylabel("Y values")
    plt.savefig(f'{Data.plt_dir}/scan_{energy:03}.png')
    plt.close()
# -----------------------------------------------
