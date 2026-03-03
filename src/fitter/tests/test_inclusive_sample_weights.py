'''
Module with tests for the Reader class of the inclusive_sample_weights module
'''


import numpy
import pandas as pnd

from rx_common                       import Component
from dmu                             import LogStore
from fitter.inclusive_sample_weights import Reader

log=LogStore.add_logger('fitter:test_inclusive_sample_weights')

ccbar_components = [Component.bpjpsixee, Component.bdjpsixee, Component.bsjpsixee]
#------------------------------------
def _get_df() -> pnd.DataFrame:
    d_data         = {'proc' : [], 'b' : []}
    d_data['proc'] = numpy.random.choice(ccbar_components, size=10).tolist()
    d_data['b']    = numpy.random.normal(0, 1, size=10).tolist()

    return pnd.DataFrame(d_data)
#------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    df            = _get_df()
    obj           = Reader(df)
    df['wgt_sam'] = obj.get_weights()
    df            = df.sort_values(by='proc')

    print(df)
#------------------------------------
