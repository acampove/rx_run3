'''
Module used to store utility functions
'''
from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_classifier:utilities')
#---------------------------------
def add_muon_columns(rdf : RDataFrame) -> RDataFrame:
    '''
    Defines columns in dataframe, needed only for muon channel:

    - Columns with brem correction, that exist in electron channel, but in the muon channel are the same as the default ones
    '''

    l_var = [
            'B_PT',
            'Jpsi_PT',
            'B_DIRA_OWNPV',
            'Jpsi_DIRA_OWNPV',
            'L1_PT',
            'L2_PT',
            'B_M',
            ]

    for var in l_var:
        # The underscore is needed due to:
        #
        # - ROOT does not allow for branches with periods
        # - Periods are replaced with underscore in CVPredict tool anyway.
        rdf = rdf.Define(f'brem_track_2_{var}_brem_track_2', var)

    return rdf
#---------------------------------
