'''
Module testing the MassCalculator class
'''
import os
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                    import RDataFrame, RDF
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_data.mass_calculator import MassCalculator

log=LogStore.add_logger('rx_data:test_mass_calculator')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    user     = os.environ['USER']
    plot_dir = f'/tmp/{user}/tests/rx_data/mass_calculator'
# ----------------------
def _validate_rdf(
        rdf_in : RDataFrame|RDF.RNode,
        rdf_ot : RDataFrame|RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf_in: Original dataframe
    rdf_ot: DataFrame with EVENTNUMBER, RUNNUMBER and masses
    '''
    nent_in = rdf_in.Count().GetValue()
    nent_ot = rdf_ot.Count().GetValue()

    assert nent_in == nent_ot

    s_col = { name.c_str() for name in rdf_ot.GetColumnNames() }
    assert s_col == {'EVENTNUMBER', 'RUNNUMBER', 'B_Mass_kkk', 'B_Mass_kpipi', 'B_M', 'B_Mass_check'}

    data = rdf_ot.AsNumpy(['B_M', 'B_Mass_kpipi', 'B_Mass_kkk', 'B_Mass_check'])
    df   = pnd.DataFrame(data)

    df.plot.hist(range=(3000, 7000), bins=100, histtype='step')
    plt.axvline(x=5280, label='PDG', linestyle=':')
    plt.show()
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
