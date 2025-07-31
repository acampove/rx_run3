'''
Module testing the MassCalculator class
'''

from ROOT                    import RDataFrame, RDF
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_data.mass_calculator import MassCalculator

log=LogStore.add_logger('rx_data:mass_calculatr')
# ----------------------
def _validate_rdf(rdf : RDataFrame|RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf: DataFrame with EVENTNUMBER, RUNNUMBER and masses
    '''
    rdf.Display().Print()
# ----------------------
def test_hadronic():
    '''
    Will run test where
    Kee -> KKK   in B_Mass_kkk
    Kee -> Kpipi in B_Mass_kpipi
    '''
    with RDFGetter.max_entries(value=10_000):
        gtr = RDFGetter(
            sample ='Bu_Kee_eq_btosllball05_DPC',
            trigger='Hlt2RD_BuToKpEE_MVA')
        rdf_in = gtr.get_rdf(per_file=False)

    cal    = MassCalculator(rdf=rdf_in)
    rdf_ot = cal.get_rdf()

    _validate_rdf(rdf_in=rdf_in, rdf_ot=rdf_ot)
# ----------------------
