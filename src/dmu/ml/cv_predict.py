'''
Module holding CVPredict class
'''
import pandas as pnd
import numpy

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
            return True

        return False
    # --------------------------------------------
    def _predict_with_overlap(self, df_ft):
        '''
        Takes pandas dataframe with features

        Will return numpy array of prediction probabilities when there is an overlap
        of data and model hashes
        '''
        # pylint: disable = too-many-locals
        df_ft = ut.index_with_hashes(df_ft)

        s_dat_hash = set(df_ft.index)

        d_prob        = {}

        ntotal = len(s_dat_hash)
        log.debug(f'Total size: {ntotal}')
        for model in self._l_model:
            s_mod_hash = model.hashes
            s_hash     = s_dat_hash.difference(s_mod_hash)
            l_hash     = list(s_hash)
            df_ft_group= df_ft.loc[df_ft.index.isin(s_hash)]
            l_prob     = model.predict_proba(df_ft_group)

            d_prob_tmp = dict(zip(l_hash, l_prob))
            d_prob.update(d_prob_tmp)

            ngroup = len(l_prob)
            log.debug(f'Hash group size: {ngroup}')

        ndata = len(df_ft)
        nprob = len(d_prob)

        if ndata != nprob:
            log.error(f'Dataset size ({ndata}) and probabilities size ({nprob}) differ')
            raise ValueError

        l_prob   = [ d_prob[hsh] for hsh in df_ft.index ]
        arr_prob = numpy.ndarray(l_prob)

        return arr_prob
    # --------------------------------------------
    def predict(self):
        '''
        Will return array of prediction probabilities for the signal category
        '''
        df_ft = self._get_df()
        model = self._l_model[0]

        if self._non_overlapping_hashes(model, df_ft):
            log.debug('No intersecting hashes found between model and data')
            l_prb = model.predict_proba(df_ft)
        else:
            log.info('Intersecting hashes found between model and data')
            l_prb = self._predict_with_overlap(df_ft)

        return l_prb
# ---------------------------------------
