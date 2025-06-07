'''
Script with functions meant to test CVPerformance
'''

import dmu.testing.utilities as tut
from dmu.stats.cv_performance import CVPerformance

# -------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/stats/cv_performance'
# -------------------------------
def test_simple():
    '''
    Simplest test
    '''
    rdf_sig_1 = tut.get_rdf(kind='sig_1')
    rdf_bkg_1 = tut.get_rdf(kind='bkg_1')
    rdf_bkg_2 = tut.get_rdf(kind='bkg_2')

    l_model_1 = tut.get_models(rdf_sig_1, rdf_bkg_1)
    l_model_2 = tut.get_models(rdf_sig_1, rdf_bkg_2)

    cvp = CVPerformance()
    cvp.add_data(sig=rdf_sig_1, bkg=rdf_bkg_1, model=l_model_1, name='def')
    cvp.add_data(sig=rdf_sig_1, bkg=rdf_bkg_2, model=l_model_2, name='alt')
    cvp.save(name=f'{Data.out_dir}/simple')
# -------------------------------
