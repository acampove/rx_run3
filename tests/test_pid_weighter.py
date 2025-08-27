'''
Module containing functions used to test PIDWeighter
'''
import numpy
from ROOT                  import RDF, RDataFrame # type: ignore
from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_misid.pid_weighter import PIDWeighter

log=LogStore.add_logger('rx_misid:test_pid_weighter')
# ----------------------
def _validate_weights(wgt : numpy.ndarray, rdf : RDataFrame|RDF.RNode) -> None:
    '''
    Parameters
    -------------
    wgt: Numpy array of weights
    rdf: ROOT dataframe
    '''
    nentries = rdf.Count().GetValue()
    assert nentries == wgt.shape[0]
# ----------------------
def _get_rdf() -> RDataFrame|RDF.RNode:
    '''
    Returns
    -------------
    ROOT dataframe with simulation
    '''
    gtr = RDFGetter(
        sample  ='Bu_JpsiPi_ee_eq_DPC',
        trigger ='Hlt2RD_BuToKpEE_MVA_noPID',
        analysis='nopid')

    rdf = gtr.get_rdf(per_file=False)

    return rdf
# ----------------------
def test_simple() -> None:
    '''
    Simplest test
    '''
    rdf = _get_rdf()
    cfg = {}

    wtr = PIDWeighter(rdf=rdf, cfg=cfg)
    arr_wgt = wtr.get_weights(region='control')

    _validate_weights(wgt=arr_wgt, rdf=rdf)
