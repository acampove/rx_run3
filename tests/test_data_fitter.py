'''
Module meant to test DataFitter class
'''

from dmu.workflow.cache import Cache
from dmu.generic        import utilities  as gut
from fitter.data_fitter import DataFitter

# -------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    cfg = gut.load_conf(
            package='fitter_data',
            fpath  ='tests/data_fit.yaml')
    with Cache.turn_off_cache(val=True):
        ftr = DataFitter(
                sample = '',
                trigger= '',
                project= '',
                q2bin  = '',
                cfg    = cfg)
        res = ftr.run()
# -------------------------------------------
