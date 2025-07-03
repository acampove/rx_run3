'''
This file contains tests for MisIDFitter
'''

from dmu.stats.zfit         import zfit
from rx_misid.misid_dataset import MisIDDataset
from rx_misid.misid_fitter  import MisIDFitter

# ---------------------------------------------------
class Data:
    '''
    Used to store attributes
    '''
    obs = zfit.Space('mass', limits=(4500, 7000))
# ---------------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    q2bin = 'low'

    obj  = MisIDDataset(q2bin=q2bin)
    d_df = obj.get_data(only_data=True)
    df   = d_df['data']

    data = zfit.data.Data.from_pandas(df=df, obs=Data.obs)

    ftr  = MisIDFitter(data=data)
    pdf  = ftr.get_pdf()
