'''
This file contains tests for MisIDFitter
'''

from dmu.stats.zfit         import zfit
from dmu.stats              import utilities  as sut
from zfit.core.interfaces   import ZfitData   as zdata
from rx_misid.misid_fitter  import MisIDFitter

# ---------------------------------------------------
class Data:
    '''
    Used to store attributes
    '''
    obs = zfit.Space('mass', limits=(4500, 7000))
# ---------------------------------------------------
def _get_toy_data() -> zdata:
    pdf = sut.get_model(kind='s+b')
    sam = pdf.create_sampler(n=1000)

    return sam
# ---------------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    q2bin = 'low'
    data  = _get_toy_data()

    ftr   = MisIDFitter(data=data, q2bin=q2bin)
    pdf   = ftr.get_pdf()
