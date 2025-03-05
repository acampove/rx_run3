'''
Module holding brem bias corrector class
'''
from vector                 import MomentumObject4D as v4d

from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_data:brem_bias_corrector')
# --------------------------
class BremBiasCorrector:
    '''
    Class meant to correct bias of brem energy
    '''
    # --------------------------
    def __init__(self):
        pass
    # --------------------------
    def correct(self, brem : v4d, row : int, col : int) -> v4d:
        '''
        Takes 4 vector with brem, the row and column locations in ECAL
        Returns corrected photon
        '''
        return brem
# --------------------------
