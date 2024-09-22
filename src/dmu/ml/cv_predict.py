'''
Module holding CVPredict class
'''
import pandas as pnd

from ROOT import RDataFrame

import dmu.ml.utilities as ut

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:cv_predict')
# ---------------------------------------
class CVPredict:
    '''
    Class used to get classification probabilities from ROOT
    dataframe and a set of models. The models were trained with CVClassifier
    '''
    def __init__(self, models : list | None = None, rdf : RDataFrame | None = None):
        '''
        Will take a list of CVClassifier models and a ROOT dataframe
        '''

        if models is None:
            raise ValueError('No list of models passed')

        if rdf is None:
            raise ValueError('No ROOT dataframe passed')

        self._l_model = models
        self._rdf     = rdf
    # --------------------------------------------
    def _get_df(self):
        '''
        Will make ROOT rdf into dataframe and return it
        '''
        model = self._l_model[0]
        l_ft  = model.features
        d_data= self._rdf.AsNumpy(l_ft)
        df_ft = pnd.DataFrame(d_data)

        return df_ft
    # --------------------------------------------
    def _non_overlapping_hashes(self, model, df_ft):
        '''
        Will return True if hashes of model and data do not overlap
        '''

        s_mod_hash = model.hashes
        s_dff_hash = ut.get_hashes(df_ft)

        s_int = s_mod_hash.intersection(s_dff_hash)
        if len(s_int) == 0:
            log.debug('No intersecting hashes found between model and data')
            return True

        return False
    # --------------------------------------------
    def predict(self):
        '''
        Will return array of prediction probabilities for the signal category
        '''
        df_ft = self._get_df()
        model = self._l_model[0]

        if self._non_overlapping_hashes(model, df_ft):
            l_prb = model.predict_proba(df_ft)
            return l_prb

        return
# ---------------------------------------
