'''
Module testing the MassCalculator class
'''
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_data.mass_calculator import MassCalculator

log=LogStore.add_logger('rx_data:mass_calculatr')
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    gtr = RDFGetter(
        sample='Bu_Kee_eq_btosllball05_DPC',
        trigger='Hlt2RD_BuToKpEE_MVA')
    rdf = gtr.get_rdf(per_file=False)
    rdf = rdf.Range(10)

    cal = MassCalculator(rdf=rdf)
    rdf = cal.get_rdf()

    rdf.Display().Print()

