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
from rx_data.swp_calculator import SWPCalculator
from rx_data.mis_calculator import MisCalculator

# ----------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    inp_dir : str = '/home/acampove/external_ssd/Data/main/v5'
    out_dir : str = '/tmp/rx_data/tests/swap_calculator'

    l_file_wc = [
            #'data_24_mag*_24c1_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            'data_24_mag*_24c2_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            #'data_24_mag*_24c3_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            #'data_24_mag*_24c4_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            ]
# ----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------------
def _get_rdf(test : str, file_wc : str) -> RDataFrame:
    if test == 'cascade':
        json_path = files('rx_data_data').joinpath('tests/swap_adder/bpd0kpienu.json')
        df        = pnd.read_json(json_path)
        d_data    = df.to_dict(orient='list')
        d_numpy   = { name : numpy.array(l_val) for name, l_val in d_data.items() }
        rdf       = RDF.FromNumpy(d_numpy)
        rdf       = rdf.Define('EVENTNUMBER', '1')
        rdf       = rdf.Define('RUNNUMBER'  , '2')

        return rdf

    if test == 'jpsi_misid':
        rdf   = RDataFrame('DecayTree', f'{Data.inp_dir}/{file_wc}')
        rdf   = rdf.Filter('Jpsi_M * Jpsi_M > 15000000')
        msc   = MisCalculator(rdf=rdf, trigger='Hlt2RD_BuToKpMuMu_MVA')
        rdf   = msc.get_rdf()

        return rdf

    raise ValueError(f'Invalid test: {test}')
# ----------------------------------
def test_cascade():
    '''
    Tests cascade decay contamination
    '''
    rdf = _get_rdf(test='cascade', file_wc='NA')
    obj = SWPCalculator(rdf, d_lep={'L1' : 211, 'L2' : 211}, d_had={'H' : 321})
    rdf = obj.get_rdf(preffix='cascade')

    _plot(rdf, 'cascade', preffix='cascade')
# ----------------------------------
@pytest.mark.parametrize('file_wc', Data.l_file_wc)
def test_jpsi_misid(file_wc : str):
    '''
    Tests jpsi misid contamination
    '''
    rdf = _get_rdf(test='jpsi_misid', file_wc = file_wc)
    obj = SWPCalculator(rdf, d_lep={'L1' : 13, 'L2' : 13}, d_had={'H' : 13})
    rdf = obj.get_rdf(preffix='jpsi_misid')

    _plot(rdf, file_wc, preffix='jpsi_misid')
# ----------------------------------
def _plot(rdf : RDataFrame, file_wc : str, preffix : str):
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

    suffix = file_wc.replace('*', 'p')
    plt.savefig(f'{Data.out_dir}/{preffix}_{suffix}.png')
    plt.close('all')
# ----------------------------------
