'''
Module storing ElectronBiasCorrector class
'''

import numpy
import pandas as pnd

from ROOT                  import RDataFrame, RDF
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
    def _calculate_correction(self, row):
        return 1
    # ------------------------------------------
    def _pick_column(self, name : str) -> bool:
        col_type = self._rdf.GetColumnType(name)
        if 'RVec' in col_type:
            return False

        if col_type == 'Bool_t':
            return False

        if 'Hlt' in name:
            return False

        if 'DTF' in name:
            return False

        if name in ['EVENTNUMBER', 'RUNNUMBER', 'B_M']:
            return True

        if name.startswith('L1_'):
            #log.info(f'{col_type:<20}{name}')
            return True

        if name.startswith('L2_'):
            #log.info(f'{col_type:<20}{name}')
            return True

        return False
    # ------------------------------------------
    def _df_from_rdf(self):
        l_col  = [ name.c_str() for name in self._rdf.GetColumnNames() if self._pick_column(name.c_str()) ]
        d_data = self._rdf.AsNumpy(l_col)
        df     = pnd.DataFrame(d_data)

        return df
    # ------------------------------------------
    def _get_corrected_rdf(self) -> RDataFrame:
        df             = self._df_from_rdf()
        df['B_M_corr'] = df.apply(self._calculate_correction, axis=1)

        to_keep = ['EVENTNUMBER', 'B_M', 'B_M_corr']
        df  = df[to_keep]

        rdf = RDF.FromPandas(df)

        return rdf
    # ------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns corrected ROOT dataframe
        '''
        log.info('Applying bias correction')

        rdf    = self._get_corrected_rdf()

        return rdf
# ------------------------------------------
