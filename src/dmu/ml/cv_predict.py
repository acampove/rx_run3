'''
Module holding CVPredict class
'''
from typing import Optional

import pandas as pnd
import numpy
import tqdm

from ROOT import RDataFrame

import dmu.ml.utilities     as ut

from dmu.ml.cv_classifier  import CVClassifier
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

        self._l_model   = models
        self._rdf       = rdf
        self._d_nan_rep : dict[str,str]

        self._arr_patch : numpy.ndarray
    # --------------------------------------------
    def _initialize(self):
        self._rdf       = self._define_columns(self._rdf)
        self._d_nan_rep = self._get_nan_replacements()
    # --------------------------------------------
    def _define_columns(self, rdf : RDataFrame) -> RDataFrame:
        cfg = self._l_model[0].cfg

        if 'define' not in cfg['dataset']:
            log.debug('No define section found in config, will not define extra columns')
            return self._rdf

        d_def = cfg['dataset']['define']
        log.debug(60 * '-')
        log.info('Defining columns in RDF before evaluating classifier')
        log.debug(60 * '-')
        for name, expr in d_def.items():
            log.debug(f'{name:<20}{"<---":20}{expr:<100}')
            rdf = rdf.Define(name, expr)

        return rdf
    # --------------------------------------------
    def _get_nan_replacements(self) -> dict[str,str]:
        cfg = self._l_model[0].cfg

        if 'nan' not in cfg['dataset']:
            log.debug('No define section found in config, will not define extra columns')
            return {}

        return cfg['dataset']['nan']
    # --------------------------------------------
    def _replace_nans(self, df : pnd.DataFrame) -> pnd.DataFrame:
        if len(self._d_nan_rep) == 0:
            log.debug('Not doing any NaN replacement')
            return df

        log.info(60 * '-')
        log.info('Doing NaN replacements')
        log.info(60 * '-')
        for var, val in self._d_nan_rep.items():
            log.info(f'{var:<20}{"--->":20}{val:<20.3f}')
            df[var] = df[var].fillna(val)

        return df
    # --------------------------------------------
    def _get_df(self):
        '''
        Will make ROOT rdf into dataframe and return it
        '''
        model = self._l_model[0]
        l_ft  = model.features
        d_data= self._rdf.AsNumpy(l_ft)
        df_ft = pnd.DataFrame(d_data)
        df_ft = self._replace_nans(df_ft)
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
        log.debug(f'{ndif:<10}{"=":5}{ndat:<10}{"-":5}{nmod:<10}')

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
        log.warning(f'Patching {nentries} probabilities with -1')
        arr_prb[self._arr_patch] = -1

        return arr_prb
    # --------------------------------------------
    def predict(self) -> numpy.ndarray:
        '''
        Will return array of prediction probabilities for the signal category
        '''
        self._initialize()

        df_ft = self._get_df()
        model = self._l_model[0]

        if self._non_overlapping_hashes(model, df_ft):
            log.debug('No intersecting hashes found between model and data')
            arr_prb = model.predict_proba(df_ft)
        else:
            log.info('Intersecting hashes found between model and data')
            arr_prb = self._predict_with_overlap(df_ft)

        arr_prb = self._patch_probabilities(arr_prb)
        arr_prb = arr_prb.T[1]

        return arr_prb
# ---------------------------------------
