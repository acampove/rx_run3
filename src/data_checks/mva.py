'''
Module with Mva class
'''
import os

import joblib
import pandas as pnd
import numpy

from sklearn.ensemble        import GradientBoostingClassifier
from sklearn.model_selection import cross_val_score, StratifiedKFold

from log_store       import log_store

log = log_store.add_logger('data_checks:mva')
# ---------------------------------------------
class Mva:
    '''
    Class used to train classifier
    '''
    # ---------------------------------------------
    def __init__(self, dt=None, mc=None, cfg : dict | None = None):
        '''
        dt (ROOT dataframe): Holds real data
        mc (ROOT dataframe): Holds simulation
        cfg (dict)         : Dictionary storing configuration for training
        '''
        self._rdf_dt = dt
        self._rdf_mc = mc
        self._cfg    = cfg if cfg is not None else {}

        self._l_ft_name = ['x', 'y']

        self._initialized = False
    # ---------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        mod = self._train()
        self._save_model(mod)

        self._initialized = True
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

        d_ft_dt = self._rdf_dt.AsNumpy(self._l_ft_name)
        d_ft_mc = self._rdf_mc.AsNumpy(self._l_ft_name)

        df_dt = pnd.DataFrame(d_ft_dt)
        df_mc = pnd.DataFrame(d_ft_mc)

        df    = pnd.concat([df_dt, df_mc], axis=0)

        log.info(f'Using features with shape: {df.shape}')

        return df
    # ---------------------------------------------
    def _get_labels(self):
        '''
        Returns labels, 0 for background, 1 for signal
        '''
        n_dt  = self._rdf_dt.Count().GetValue()
        n_mc  = self._rdf_mc.Count().GetValue()
        l_flg = n_dt * [0] + n_mc * [1]

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
    def get_rdf(self, mva_col=None, kind=None):
        '''
        Returns ROOT dataframe with classifier branch added

        mva_col (str) : Name of branch where score is stored
        kind    (str) : Data (dt) or simulation (mc) dataframe
        '''

        self._initialize()

        return
# ---------------------------------------------
