'''
Module holding CVPredict class
'''
import pandas as pnd

from ROOT import RDataFrame

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
    def predict(self):
        '''
        Will return array of prediction probabilities for the signal category
        '''

        model = self._l_model[0]
        l_ft  = model.features
        d_data= self._rdf.AsNumpy(l_ft)
        df_ft = pnd.DataFrame(d_data)

        l_prb = model.predict_proba(df_ft)

        return l_prb
# ---------------------------------------
