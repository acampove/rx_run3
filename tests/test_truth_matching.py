'''
Module needed to test truth matching
'''

import pytest
from rx_selection           import truth_matching as tm

# TODO: Add more samples
# --------------------------
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    #'Bu_KplKplKmn_eq_sqDalitz_DPC', Removed for now
    'Bu_piplpimnKpl_eq_sqDalitz_DPC'])
def test_truth_matching(sample : str):
    '''
    Tests truth matching
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA_noPID'
    gtr = RDFGetter(sample=sample, trigger=trigger, analysis='nopid')
    rdf = gtr.get_rdf(per_file=False)

    cut = tm.get_truth(sample)
    ini = rdf.Count().GetValue()
    rdf = rdf.Filter(cut, 'truth match')
    fin = rdf.Count().GetValue()

    rep = rdf.Report()
    rep.Print()

    assert 20 * fin > ini

