'''
Module with TrainMva class
'''
import os

import joblib
import pandas as pnd
import numpy

from sklearn.ensemble        import GradientBoostingClassifier
from sklearn.model_selection import cross_val_score, StratifiedKFold

from log_store       import log_store

log = log_store.add_logger('data_checks:train_mva')
# ---------------------------------------------
class TrainMva:
    '''
    Interface to scikit learn used to train classifier
    '''
    # ---------------------------------------------
    def __init__(self, bkg=None, sig=None, cfg : dict | None = None):
        '''
        bkg (ROOT dataframe): Holds real data
        sig (ROOT dataframe): Holds simulation
        cfg (dict)         : Dictionary storing configuration for training
        '''
        self._rdf_bkg = bkg
        self._rdf_sig = sig
        self._cfg    = cfg if cfg is not None else {}

        self._l_ft_name = None
        self._model     = None
    # ---------------------------------------------
    def _get_model(self):
        '''
        Will return model, either from earlier training, or will train all over again
        '''
        model_path = self._cfg['saving']['path']
        if not os.path.isfile(model_path):
            log.info('Model not found, training')
            model = self._train()
            return model

        log.info('Model found, loading')
        model = joblib.load(model_path)

        return model
    # ---------------------------------------------
    def _save_model(self, model):
        model_path = self._cfg['saving']['path']
        if os.path.isfile(model_path):
            log.info(f'Model found in {model_path}, not saving')
            return

        dir_name = os.path.dirname(model_path)
        os.makedirs(dir_name, exist_ok=True)

        log.info(f'Saving model to: {model_path}')
        joblib.dump(model, model_path)
    # ---------------------------------------------
    def _get_features(self):
        '''
        Returns pandas dataframe with features
        '''

        d_ft_bkg = self._rdf_bkg.AsNumpy(self._l_ft_name)
        d_ft_sig = self._rdf_sig.AsNumpy(self._l_ft_name)

        df_bkg = pnd.DataFrame(d_ft_bkg)
        df_sig = pnd.DataFrame(d_ft_sig)

        df    = pnd.concat([df_bkg, df_sig], axis=0)

        log.info(f'Using features with shape: {df.shape}')

        return df
    # ---------------------------------------------
    def _get_labels(self):
        '''
        Returns labels, 0 for background, 1 for signal
        '''
        n_bkg  = self._rdf_bkg.Count().GetValue()
        n_sig  = self._rdf_sig.Count().GetValue()
        l_flg = n_bkg * [0] + n_sig * [1]

        arr_flg = numpy.array(l_flg)

        log.info(f'Using labels with shape: {arr_flg.shape}')

        return arr_flg
    # ---------------------------------------------
    def _train(self):
        '''
        Will create model, train it and return it
        '''
        df_ft = self._get_features()
        l_flg = self._get_labels()
        nfold = self._cfg['training']['nfold']

        model = GradientBoostingClassifier()
        kfold = StratifiedKFold(n_splits=nfold)
        cvscr = cross_val_score(model, df_ft, l_flg, cv=kfold)

        model.fit(df_ft, l_flg)
        model.scores = cvscr

        return model
    # ---------------------------------------------
    def run(self):
        '''
        Will do the training
        '''

        self._l_ft_name = self._cfg['training']['features']

        mod = self._get_model()
        self._save_model(mod)
        self._model = mod
# ---------------------------------------------
