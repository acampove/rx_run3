'''
Module storing ElectronBiasCorrector class
'''

import numpy
from ROOT import RDataFrame, RDF

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:electron_bias_corrector')
# ------------------------------------------
class ElectronBiasCorrector:
    '''
    Class meant to correct B mass without DTF constraint
    by correcting biases in electrons
    '''
    # ------------------------------------------
    def __init__(self, rdf : RDataFrame):
        self._rdf = rdf
    # ------------------------------------------
    def _get_corrected_mass(self) -> numpy.ndarray:
        arr_mass = self._rdf.AsNumpy(['B_M'])['B_M']

        return arr_mass
    # ------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns corrected ROOT dataframe
        '''
        log.info('Applying bias correction')

        d_data        = self._rdf.AsNumpy(['EVENTNUMBER', 'RUNNUMBER'])
        d_data['B_M'] = self._get_corrected_mass()

        rdf = RDF.FromNumpy(d_data)

        return rdf
# ------------------------------------------
