'''
File with tests for PIDWeigher class
'''
import pytest

from dmu.stats.zfit        import zfit
from rx_data.rdf_getter    import RDFGetter
from rx_misid.pid_weighter import PIDWeighter

# ------------------------------------
class Data:
    '''
    Stores shared attributes
    '''
    obs     = zfit.Space('B_Mass_smr', limits=(4500, 7000))
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
# ------------------------------------
@pytest.mark.parametrize('sample', ['Bu_KplKplKmn_eq_sqDalitz_DPC'])
def test_simple(sample : str):
    '''
    Simplest test
    '''
    gtr = RDFGetter(sample=sample, trigger=Data.trigger, analysis='nopid')
    rdf = gtr.get_rdf()

    wtr = PIDWeighter(rdf=rdf)
    pdf = wtr.get_pdf(obs=Data.obs)
