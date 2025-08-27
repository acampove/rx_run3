'''
Module containing PIDWeighter class
'''

import numpy
from omegaconf import DictConfig
from ROOT      import RDF, RDataFrame # type: ignore

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_misid:pid_weighter')
# ------------------------------------------
class PIDWeighter:
    '''
    This class is meant to take ROOT dataframes for simulation and return, PID weights
    for any region, control, signal, etc
    '''
    # ----------------------
    def __init__(self, rdf : RDataFrame|RDF.RNode, cfg : DictConfig) -> None:
        '''
        Parameters
        -------------
        rdf: ROOT dataframe with data whose weights are needed
        cfg: omegaconf configuration object to configure weights calculation
        '''
        self._rdf = rdf
        self._cfg = cfg
    # ----------------------
    def get_weights(self, region : str) -> numpy.ndarray:
        '''
        Parameters
        -------------
        region : E.g. signal, control

        Returns
        -------------
        1D array of weights for candidate
        '''
        nentries = self._rdf.Count().GetValue()

        return numpy.ones(nentries)
# ------------------------------------------
