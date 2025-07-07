'''
Module meant to test DataFitter class
'''
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

    ftr = DataFitter(
            sample = '',
            trigger= '',
            project= '',
            q2bin  = '',
            cfg    = cfg)
    res = ftr.run()
# -------------------------------------------
