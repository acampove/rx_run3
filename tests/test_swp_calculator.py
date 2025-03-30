'''
Module with tests for swap calculator class
'''
import os
from importlib.resources     import files

import numpy
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from ROOT                   import RDataFrame, RDF
from dmu.logging.log_store  import LogStore
from rx_data.rdf_getter     import RDFGetter
from rx_data.swp_calculator import SWPCalculator

log = LogStore.add_logger('rx_data:test_swp_calculator')
# ----------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    out_dir : str = '/tmp/tests/rx_data/swap_calculator'
# ----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
    LogStore.set_level('rx_data:swp_calculator', 20)
# ----------------------------------
def _get_rdf(kind : str) -> RDataFrame:
    if   kind == 'dt_ss':
        sample = 'DATA_24_MagUp_24c3'
        trigger= 'Hlt2RD_BuToKpEE_SameSign_MVA'
    elif kind == 'mc':
        sample = 'Bu_JpsiK_ee_eq_DPC'
        trigger= 'Hlt2RD_BuToKpEE_MVA'
    else:
        raise ValueError(f'Invalid dataset of kind: {kind}')

    data_dir = os.environ['DATADIR']
    RDFGetter.samples = {'main' : f'{data_dir}/samples/main.yaml'}

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()
    rdf = rdf.Range(100_000)

    return rdf
# ----------------------------------
@pytest.mark.parametrize('kind', ['mc', 'dt_ss'])
def test_cascade(kind : str):
    '''
    Tests cascade decay contamination
    '''
    rdf = _get_rdf(kind=kind)
    obj = SWPCalculator(rdf, d_lep={'L1' : 211, 'L2' : 211}, d_had={'H' : 321})
    rdf = obj.get_rdf(preffix='cascade', progress_bar=True, use_ss= 'ss' in kind)

    _plot(rdf, preffix='cascade', kind=kind)
# ----------------------------------
@pytest.mark.parametrize('kind', ['mc', 'dt_ss'])
def test_jpsi_misid(kind : str):
    '''
    Tests jpsi misid contamination
    '''
    rdf = _get_rdf(kind=kind)
    obj = SWPCalculator(rdf, d_lep={'L1' : 13, 'L2' : 13}, d_had={'H' : 13})
    rdf = obj.get_rdf(preffix='jpsi_misid', progress_bar=True, use_ss= 'ss' in kind)

    _plot(rdf, preffix='jpsi_misid', kind=kind)
# ----------------------------------
def _plot(rdf : RDataFrame, preffix : str, kind : str):
    d_data = rdf.AsNumpy([f'{preffix}_mass_swp', f'{preffix}_mass_org'])
    arr_swp= d_data[f'{preffix}_mass_swp']
    arr_org= d_data[f'{preffix}_mass_org']

    mass_rng = {'jpsi_misid' : [2700, 3300], 'cascade' : [1800, 1950]}[preffix]

    plt.hist(arr_swp, bins=40, range=mass_rng, histtype='step', label='Swapped')
    plt.hist(arr_org, bins=40, range=mass_rng, histtype='step', label='Original')
    plt.grid(False)
    plt.legend()

    if preffix == 'jpsi_misid':
        plt.axvline(x=3100, color='r', label=r'$J/\psi$')
    else:
        plt.axvline(x=1864, color='r', label='$D_0$')

    plt.savefig(f'{Data.out_dir}/{preffix}_{kind}.png')
    plt.close('all')
# ----------------------------------
