'''
Module meant to test DataFitter class
'''
import pytest

from dmu.workflow.cache import Cache
from dmu.generic        import utilities  as gut
from rx_data.rdf_getter import RDFGetter
from rx_selection       import selection  as sel
from fitter.data_fitter import DataFitter

# -------------------------------------------
def test_toy():
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='tests/data_toy.yaml')
    with Cache.turn_off_cache(val=True):
        ftr = DataFitter(
            sample = 'data_toy',
            trigger= '',
            project= '',
            q2bin  = '',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_reso_muon():
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/muon/data.yaml')

    with Cache.turn_off_cache(val=False), \
         sel.custom_selection(d_sel = {'bdt' : '(1)'}), \
         RDFGetter.max_entries(value=100_000):

        ftr = DataFitter(
            sample = 'DATA_24_MagDown_24c2',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['central'])
def test_rare_muon(q2bin : str):
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/muon/data.yaml')

    with Cache.turn_off_cache(val=False):
        ftr = DataFitter(
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_reso_electron():
    '''
    Test fitting resonant electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='test/electron/data.yaml')

    with Cache.turn_off_cache(val=False), \
        sel.custom_selection(d_sel={'block' : 'block == 1 || block == 2'}):

        ftr = DataFitter(
            sample = 'DATA_24_MagDown_24c2',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_rare_electron():
    '''
    Test fitting rare electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    with Cache.turn_off_cache(val=False):
        ftr = DataFitter(
                sample = 'DATA_24_*',
                trigger= 'Hlt2RD_BuToKpEE_MVA',
                project= 'rx',
                q2bin  = 'central',
                cfg    = cfg)
        ftr.run()
# -------------------------------------------
