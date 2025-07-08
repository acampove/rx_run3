'''
Module meant to test DataFitter class
'''

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
        res = ftr.run()
# -------------------------------------------
def test_muon():
    '''
    Test using toy data
    '''
    cfg = gut.load_conf(
        package='fitter_data',
        fpath  ='tests/data_muon.yaml')

    with Cache.turn_off_cache(val=True), \
         sel.custom_selection(d_sel = {'bdt' : '(1)'}), \
         RDFGetter.max_entries(value=300_000):

        ftr = DataFitter(
                sample = 'DATA_24_MagDown_24c2',
                trigger= 'Hlt2RD_BuToKpMuMu_MVA',
                project= 'rx',
                q2bin  = 'jpsi',
                cfg    = cfg)
        res = ftr.run()
# -------------------------------------------
