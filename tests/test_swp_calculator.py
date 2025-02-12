'''
Module with tests for swap calculator class
'''
import os
from importlib.resources     import files

import pytest
import pandas            as pnd
import matplotlib.pyplot as plt
from ROOT                   import RDataFrame, EnableImplicitMT
from rx_data.swp_calculator import SWPCalculator
from rx_data.mis_calculator import MisCalculator

# ----------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    nthread = 10
    EnableImplicitMT(nthread)

    inp_dir : str = '/home/acampove/external_ssd/Data/main/v5'
    out_dir : str = '/tmp/rx_data/tests/swap_calculator'

    l_file_wc = [
            'data_24_mag*_24c1_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            'data_24_mag*_24c2_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            'data_24_mag*_24c3_Hlt2RD_BuToKpMuMu_MVA_0000000000.root',
            'data_24_mag*_24c4_Hlt2RD_BuToKpMuMu_MVA_0000000000.root']
# ----------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    os.makedirs(Data.out_dir, exist_ok=True)
# ----------------------------------
def _get_df(test : str, file_wc : str) -> pnd.DataFrame:
    if test == 'cascade':
        json_path = files('rx_data_data').joinpath('tests/swap_adder/bpd0kpienu.json')
        df = pnd.read_json(json_path)
        return df

    if test == 'jpsi_misid':
        rdf   = RDataFrame('DecayTree', f'{Data.inp_dir}/{file_wc}')
        rdf   = rdf.Filter('Jpsi_M * Jpsi_M > 15000000')
        msc   = MisCalculator(rdf=rdf, trigger='Hlt2RD_BuToKpMuMu_MVA')
        rdf   = msc.get_rdf()

        d_data= rdf.AsNumpy()
        df    = pnd.DataFrame(d_data)
        return df

    raise ValueError(f'Invalid test: {test}')
# ----------------------------------
def test_cascade():
    '''
    Tests cascade decay contamination
    '''
    df  = _get_df(test='cascade')

    obj = SWPCalculator(df, d_lep={'L1' : 211, 'L2' : 211}, d_had={'H' : 321})
    df  = obj.get_df()
    df.H_swp.hist(bins=40, range=(1800, 1950) , histtype='step', label='Swapped')
    df.H_org.hist(bins=40, range=(1800, 1950) , histtype='step', label='Original')
    plt.axvline(x=1864, color='r', label='$D_0$')
    plt.grid(False)
    plt.legend()

    plt.savefig(f'{Data.out_dir}/cascade.png')
    plt.close('all')
# ----------------------------------
@pytest.mark.parametrize('file_wc', Data.l_file_wc)
def test_jpsi_misid(file_wc : str):
    '''
    Tests jpsi misid contamination
    '''
    df  = _get_df(test='jpsi_misid', file_wc = file_wc)

    if len(df) == 0:
        raise ValueError('No entries found in input dataframe')

    obj = SWPCalculator(df, d_lep={'L1' : 13, 'L2' : 13}, d_had={'H' : 13})
    df  = obj.get_df(nthread=Data.nthread)
    df.H_swp.hist(bins=40, range=(2700, 3300) , histtype='step', label='Swapped')
    df.H_org.hist(bins=40, range=(2700, 3300) , histtype='step', label='Original')
    plt.axvline(x=3100, color='r', label=r'$J/\psi$')
    plt.grid(False)
    plt.legend()

    suffix = file_wc.replace('*', 'p')
    plt.savefig(f'{Data.out_dir}/jpsi_misid_{suffix}.png')
    plt.close('all')
