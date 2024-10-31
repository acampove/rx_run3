'''
Module holding CVPredict class
'''
from typing import Optional

import pandas as pnd
import numpy
import tqdm

from ROOT import RDataFrame

import dmu.ml.utilities     as ut
import dmu.ml.cv_classifier as CVClassifier

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:ml:cv_predict')
# ---------------------------------------
class CVPredict:
    '''
    Class used to get classification probabilities from ROOT
    dataframe and a set of models. The models were trained with CVClassifier
    '''
    def __init__(self, models : Optional[list] = None, rdf : Optional[RDataFrame] = None):
        '''
        Will take a list of CVClassifier models and a ROOT dataframe
        '''

        if models is None:
            raise ValueError('No list of models passed')

        if rdf is None:
            raise ValueError('No ROOT dataframe passed')

        self._l_model = models
        self._rdf     = rdf

        self._arr_patch : numpy.ndarray
    # --------------------------------------------
    def _get_df(self):
        '''
        Will make ROOT rdf into dataframe and return it
        '''
        model = self._l_model[0]
        l_ft  = model.features
        d_data= self._rdf.AsNumpy(l_ft)
        df_ft = pnd.DataFrame(d_data)
        df_ft = ut.patch_and_tag(df_ft)

        if 'patched_indices' in df_ft.attrs:
            self._arr_patch = df_ft.attrs['patched_indices']

        nfeat = len(l_ft)
        log.info(f'Found {nfeat} features')
        for name in l_ft:
            log.debug(name)

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
    def _predict_with_overlap(self, df_ft : pnd.DataFrame) -> numpy.ndarray:
        '''
        Takes pandas dataframe with features

        Will return numpy array of prediction probabilities when there is an overlap
        of data and model hashes
        '''
        df_ft      = ut.index_with_hashes(df_ft)
        d_prob     = {}
        ntotal     = len(df_ft)
        log.debug(30 * '-')
        log.info(f'Total size: {ntotal}')
        log.debug(30 * '-')
        for model in tqdm.tqdm(self._l_model, ascii=' -'):
            d_prob_tmp = self._evaluate_model(model, df_ft)
            d_prob.update(d_prob_tmp)

        ndata  = len(df_ft)
        nprob  = len(d_prob)
        if ndata != nprob:
            log.warning(f'Dataset size ({ndata}) and probabilities size ({nprob}) differ, likely there are repeated entries')

        l_prob = [ d_prob[hsh] for hsh in df_ft.index ]

        return numpy.array(l_prob)
    # --------------------------------------------
    def _evaluate_model(self, model : CVClassifier, df_ft : pnd.DataFrame) -> dict[str, float]:
        '''
        Evaluate the dataset for one of the folds, by taking the model and the full dataset
        '''
        s_dat_hash = set(df_ft.index)
        s_mod_hash = model.hashes

        s_dif_hash = s_dat_hash - s_mod_hash

        ndif = len(s_dif_hash)
        ndat = len(s_dat_hash)
        nmod = len(s_mod_hash)
        log.debug(f'{ndif:<20}{"=":10}{ndat:<20}{"-":10}{nmod:<20}')

        df_ft_group= df_ft.loc[df_ft.index.isin(s_dif_hash)]

        l_prob = model.predict_proba(df_ft_group)
        l_hash = list(df_ft_group.index)
        d_prob = dict(zip(l_hash, l_prob))
        nfeat  = len(df_ft_group)
        nprob  = len(l_prob)
        log.debug(f'{nfeat:<10}{"->":10}{nprob:<10}')

        return d_prob
    # --------------------------------------------
    def _patch_probabilities(self, arr_prb : numpy.ndarray) -> numpy.ndarray:
        if not hasattr(self, '_arr_patch'):
            return arr_prb

        nentries = len(self._arr_patch)
        log.warning(f'Patching {nentries} probabilities')
        arr_prb[self._arr_patch] = -1

        return arr_prb
    # --------------------------------------------
    def predict(self) -> numpy.ndarray:
        '''
        Will return array of prediction probabilities for the signal category
        '''
        df_ft = self._get_df()
        model = self._l_model[0]

        if self._non_overlapping_hashes(model, df_ft):
            log.debug('No intersecting hashes found between model and data')
            arr_prb = model.predict_proba(df_ft)
        else:
            log.info('Intersecting hashes found between model and data')
            arr_prb = self._predict_with_overlap(df_ft)

        arr_prb = self._patch_probabilities(arr_prb)

        return arr_prb
# ---------------------------------------
