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

log=LogStore.add_logger('rx_data:test_brem_bias_corrector')

# -----------------------------------------------
class Data:
    '''
    Data class
    '''
    plt_dir = '/tmp/tests/rx_data/bias_corrector'
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
    arr_row_edge = numpy.linspace(-3000, 3000, 50)
    arr_col_edge = numpy.linspace(-3000, 3000, 50)

    arr_row_cent = (arr_row_edge[:-1] + arr_row_edge[1:]) / 2
    arr_col_cent = (arr_col_edge[:-1] + arr_col_edge[1:]) / 2

    obj    = BremBiasCorrector()
    brem   = _get_input(energy=energy)
    l_corr = []
    for row in arr_row_cent:
        l_corr_row = []
        for col in arr_col_cent:
            brem_corr = obj.correct(brem=brem, row=row, col=col)
            energy_corr = brem_corr.e

            mu = energy / energy_corr
            if mu < 0.5 or mu > 3.0:
                log.warning(f'Found correction: {mu:.3f}')

            l_corr_row.append(mu)

        l_corr.append(l_corr_row)

    arr_corr = numpy.array(l_corr)

    plt.pcolormesh(arr_row_edge, arr_col_edge, arr_corr.T, cmap='plasma', vmin=0.9, vmax=2.0)
    plt.colorbar(label='Correction')

    os.makedirs(Data.plt_dir, exist_ok=True)

    plt.xlabel("X values")
    plt.ylabel("Y values")
    plt.savefig(f'{Data.plt_dir}/scan_{energy:03}.png')
    plt.close()
# -----------------------------------------------
