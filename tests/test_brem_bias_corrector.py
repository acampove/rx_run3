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
    df = df.sort_values(by = ['x', 'y'])

    Data.locations = df.values.tolist()
# -----------------------------------------------
def _get_input(energy : float):
    br_1 = v4d(pt=5_000, eta=3.0, phi=1.0, mass=0.511)
    br_2 = v4d(px=br_1.px, py=br_1.py, pz=br_1.pz, e=energy)

    return br_2
# -----------------------------------------------
def _plot_area(area : int, energy : float) -> None:
    brem = _get_input(energy=energy)
    obj  = BremBiasCorrector()
    l_x  = []
    l_y  = []
    l_z  = []
    for are, x, y, row, col in Data.locations:
        if are != area:
            continue

        brem_corr = obj.correct(brem=brem, row=row, col=col, area=are)
        energy_corr = brem_corr.e

        z = energy / energy_corr
        if z < 0.5 or z > 3.0:
            log.warning(f'Found correction: {z:.3f}')

        l_x.append(x)
        l_y.append(y)
        l_z.append(z)

    if area == 2:
        size = 1
    elif area == 1:
        size = 3
    elif area == 0:
        size = 18
    else:
        raise ValueError(f'Invalid area: {area}')

    plt.scatter(l_x, l_y, c=l_z, cmap='viridis', s=size, marker='s', vmin=0.9, vmax=2.0)
# -----------------------------------------------
@pytest.mark.parametrize('energy', [6_000, 8_000, 10_000, 15_000, 30_000, 50_000, 80_000])
def test_scan(energy : float):
    '''
    Will scan the calorimeter and plot corrections
    '''
    _plot_area(area=0, energy=energy)
    _plot_area(area=1, energy=energy)
    _plot_area(area=2, energy=energy)

    plt.colorbar(label='Correction')

    os.makedirs(Data.plt_dir, exist_ok=True)
    plot_path=f'{Data.plt_dir}/scan_{energy:03}.png'
    log.info(f'Saving to: {plot_path}')

    plt.xlabel("X values")
    plt.ylabel("Y values")
    plt.savefig(plot_path)
    plt.close()
# -----------------------------------------------
