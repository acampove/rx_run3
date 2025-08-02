'''
Module meant to test DataFitter class
'''
import pytest

from dmu.workflow.cache import Cache
from dmu.generic        import utilities  as gut
from rx_data.rdf_getter import RDFGetter
from rx_selection       import selection  as sel
from fitter.likelihood_factory import LikelihoodFactory

# -------------------------------------------
class Data:
    '''
    Used to share attributes
    '''
    d_block_cut = {
        'b12': 'block == 1 || block == 2',
        'b3' : 'block == 3',
        'b4' : 'block == 4',
        'b5' : 'block == 5',
        'b6' : 'block == 6',
        'b78': 'block == 7 || block == 8'}
# -------------------------------------------
def test_reso_muon():
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/muon/data.yaml')

    with Cache.turn_off_cache(val=['LikelihoodFactory']), \
         sel.custom_selection(d_sel = {'bdt' : '(1)'}), \
         RDFGetter.max_entries(value=100_000):

        ftr = LikelihoodFactory(
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

    with Cache.turn_off_cache(val=['LikelihoodFactory']):
        ftr = LikelihoodFactory(
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpMuMu_MVA',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('block', ['b12', 'b3', 'b4', 'b5', 'b6', 'b78'])
def test_reso_electron(block : str):
    '''
    Test fitting resonant electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='reso/electron/data.yaml')

    block_cut = Data.d_block_cut[block]

    with Cache.turn_off_cache(val=['LikelihoodFactory']), \
        RDFGetter.multithreading(nthreads=8),\
        sel.custom_selection(d_sel={
            'block' : block_cut,
            'brm12' : 'nbrem != 0',
            'mass'  : '(1)'}):

        ftr = LikelihoodFactory(
            name   = block,
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = 'jpsi',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_rare_electron(q2bin : str):
    '''
    Test fitting rare electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    with Cache.turn_off_cache(val=['LikelihoodFactory']),\
        sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'bdt'   : 'mva_cmb > 0.60 && mva_prc > 0.40'}):
        ftr = LikelihoodFactory(
            name   = '060_040',
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
def test_high_q2_track():
    '''
    Test fitting rare electron in high q2
    with track based cut and adding brem 0 category
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='rare/electron/data.yaml')

    with Cache.turn_off_cache(val=['LikelihoodFactory']),\
        sel.custom_selection(d_sel={
            'q2'    : 'q2_track > 14300000 && q2 < 22000000',
            'bdt'   : 'mva_cmb > 0.8 && mva_prc > 0.8'}):
        ftr = LikelihoodFactory(
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA',
            project= 'rx',
            q2bin  = 'high',
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
@pytest.mark.parametrize('q2bin', ['low', 'central', 'high'])
def test_rare_misid_electron(q2bin : str):
    '''
    Test fitting rare electron channel
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='misid/electron/data.yaml')

    l1_in_cr = '(L1_PROBNN_E < 0.2) || (L1_PID_E < 3.0)'
    l2_in_cr = '(L2_PROBNN_E < 0.2) || (L2_PID_E < 3.0)'

    with Cache.turn_off_cache(val=['LikelihoodFactory']),\
        sel.custom_selection(d_sel={
            'nobr0' : 'nbrem != 0',
            'pid_l' : f'({l1_in_cr}) || ({l2_in_cr})',
            'bdt'   : 'mva_cmb > 0.80 && mva_prc > 0.60'}):
        ftr = LikelihoodFactory(
            name   = '080_060',
            sample = 'DATA_24_*',
            trigger= 'Hlt2RD_BuToKpEE_MVA_ext',
            project= 'rx',
            q2bin  = q2bin,
            cfg    = cfg)
        ftr.run()
# -------------------------------------------
