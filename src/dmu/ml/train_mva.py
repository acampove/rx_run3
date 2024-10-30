'''
Module with TrainMva class
'''
import os
from typing import Union

import joblib
import pandas as pnd
import numpy
import matplotlib.pyplot as plt

from sklearn.metrics         import roc_curve, auc
from sklearn.model_selection import StratifiedKFold

from ROOT import RDataFrame

import dmu.ml.utilities    as ut
from dmu.ml.cv_classifier    import CVClassifier as cls
from dmu.plotting.plotter_1d import Plotter1D    as Plotter
from dmu.logging.log_store   import LogStore

log = LogStore.add_logger('data_checks:train_mva')
# ---------------------------------------------
class TrainMva:
    '''
    Interface to scikit learn used to train classifier
    '''
    # ---------------------------------------------
    def __init__(self, bkg=None, sig=None, cfg=None):
        '''
        bkg (ROOT dataframe): Holds real data
        sig (ROOT dataframe): Holds simulation
        cfg (dict)          : Dictionary storing configuration for training
        '''
        if bkg is None:
            raise ValueError('Background dataframe is not a ROOT dataframe')

        if sig is None:
            raise ValueError('Signal dataframe is not a ROOT dataframe')

        if not isinstance(cfg, dict):
            raise ValueError('Config dictionary is not a dictionary')

        self._rdf_bkg = bkg
        self._rdf_sig = sig
        self._cfg     = cfg if cfg is not None else {}

        self._l_model   : cls

        self._l_ft_name = self._cfg['training']['features']

        self._df_ft, self._l_lab = self._get_inputs()
    # ---------------------------------------------
    def _get_inputs(self) -> tuple[pnd.DataFrame, numpy.ndarray]:
        log.info('Getting signal')
        df_sig, arr_lab_sig = self._get_sample_inputs(self._rdf_sig, label = 1)

        log.info('Getting background')
        df_bkg, arr_lab_bkg = self._get_sample_inputs(self._rdf_bkg, label = 0)

        df      = pnd.concat([df_sig, df_bkg], axis=0)
        arr_lab = numpy.concatenate([arr_lab_sig, arr_lab_bkg])

        return df, arr_lab
    # ---------------------------------------------
    def _get_sample_inputs(self, rdf : RDataFrame, label : int) -> tuple[pnd.DataFrame, numpy.ndarray]:
        d_ft = rdf.AsNumpy(self._l_ft_name)
        df   = pnd.DataFrame(d_ft)
        df   = ut.cleanup(df)
        l_lab= len(df) * [label]

        return df, numpy.array(l_lab)
    # ---------------------------------------------
    def _get_model(self, arr_index : numpy.ndarray) -> cls:
        model = cls(cfg = self._cfg)
        df_ft = self._df_ft.iloc[arr_index]
        l_lab = self._l_lab[arr_index]

        log.debug(f'Training feature shape: {df_ft.shape}')
        log.debug(f'Training label size: {len(l_lab)}')

        model.fit(df_ft, l_lab)

        return model
    # ---------------------------------------------
    def _get_models(self):
        # pylint: disable = too-many-locals
        '''
        Will create models, train them and return them
        '''
        nfold = self._cfg['training']['nfold']
        rdmst = self._cfg['training']['rdm_stat']

        kfold = StratifiedKFold(n_splits=nfold, shuffle=True, random_state=rdmst)

        l_model=[]
        ifold=0
        for arr_itr, arr_its in kfold.split(self._df_ft, self._l_lab):
            log.debug(20 * '-')
            log.info(f'Training fold: {ifold}')
            log.debug(20 * '-')
            model = self._get_model(arr_itr)
            l_model.append(model)

            arr_sig_sig_tr, arr_sig_bkg_tr, arr_sig_all_tr, arr_lab_tr = self._get_scores(model, arr_itr, on_training_ok= True)
            arr_sig_sig_ts, arr_sig_bkg_ts, arr_sig_all_ts, arr_lab_ts = self._get_scores(model, arr_its, on_training_ok=False)

            self._plot_scores(arr_sig_sig_tr, arr_sig_sig_ts, arr_sig_bkg_tr, arr_sig_bkg_ts, ifold)

            self._plot_roc(arr_lab_ts, arr_sig_all_ts, arr_lab_tr, arr_sig_all_tr, ifold)

            ifold+=1

        return l_model
    # ---------------------------------------------
    def _get_scores(self, model : cls, arr_index : numpy.ndarray, on_training_ok : bool) -> tuple[numpy.ndarray, numpy.ndarray, numpy.ndarray, numpy.ndarray]:
        '''
        Returns a tuple of four arrays

        arr_sig : Signal probabilities for signal
        arr_bkg : Signal probabilities for background
        arr_all : Signal probabilities for both
        arr_lab : Labels for both
        '''
        nentries = len(arr_index)
        log.debug(f'Getting {nentries} signal probabilities')

        df_ft    = self._df_ft.iloc[arr_index]
        arr_prob = model.predict_proba(df_ft, on_training_ok=on_training_ok)
        arr_lab  = self._l_lab[arr_index]

        l_all    = [ sig_prob for [_, sig_prob] in arr_prob ]
        arr_all  = numpy.array(l_all)

        arr_sig, arr_bkg= self._split_scores(arr_prob=arr_prob, arr_label=arr_lab)

        return arr_sig, arr_bkg, arr_all, arr_lab
    # ---------------------------------------------
    def _split_scores(self, arr_prob : numpy.ndarray, arr_label : numpy.ndarray) -> tuple[numpy.ndarray, numpy.ndarray]:
        '''
        Will split the testing scores (predictions) based on the training scores

        tst is a list of lists as [p_bkg, p_sig]
        '''

        l_sig = [ prb[1] for prb, lab in zip(arr_prob, arr_label) if lab == 1]
        l_bkg = [ prb[1] for prb, lab in zip(arr_prob, arr_label) if lab == 0]

        arr_sig = numpy.array(l_sig)
        arr_bkg = numpy.array(l_bkg)

        return arr_sig, arr_bkg
    # ---------------------------------------------
    def _save_model(self, model, ifold):
        '''
        Saves a model, associated to a specific fold
        '''
        model_path = self._cfg['saving']['path']
        if os.path.isfile(model_path):
            log.info(f'Model found in {model_path}, not saving')
            return

        dir_name = os.path.dirname(model_path)
        os.makedirs(dir_name, exist_ok=True)

        model_path = model_path.replace('.pkl', f'_{ifold:03}.pkl')

        log.info(f'Saving model to: {model_path}')
        joblib.dump(model, model_path)
    # ---------------------------------------------
    def _plot_scores(self, arr_sig_trn, arr_sig_tst, arr_bkg_trn, arr_bkg_tst, ifold):
        # pylint: disable = too-many-arguments, too-many-positional-arguments
        '''
        Will plot an array of scores, associated to a given fold
        '''
        log.debug(f'Plotting scores for {ifold} fold')

        if 'val_dir' not in self._cfg['plotting']:
            log.warning('Scores path not passed, not plotting scores')
            return

        val_dir  = self._cfg['plotting']['val_dir']
        val_dir  = f'{val_dir}/fold_{ifold:03}'
        os.makedirs(val_dir, exist_ok=True)

        plt.hist(arr_sig_trn, alpha   =   0.3, bins=50, range=(0,1), color='b', density=True, label='Signal Train')
        plt.hist(arr_sig_tst, histtype='step', bins=50, range=(0,1), color='b', density=True, label='Signal Test')

        plt.hist(arr_bkg_trn, alpha   =   0.3, bins=50, range=(0,1), color='r', density=True, label='Background Train')
        plt.hist(arr_bkg_tst, histtype='step', bins=50, range=(0,1), color='r', density=True, label='Background Test')

        plt.legend()
        plt.title(f'Fold: {ifold}')
        plt.xlabel('Signal probability')
        plt.ylabel('Normalized')
        plt.savefig(f'{val_dir}/scores.png')
        plt.close()
    # ---------------------------------------------
    def _plot_roc(self,
                  l_lab_ts : numpy.ndarray,
                  l_prb_ts : numpy.ndarray,
                  l_lab_tr : numpy.ndarray,
                  l_prb_tr : numpy.ndarray,
                  ifold    : int):
        '''
        Takes the labels and the probabilities and plots ROC
        curve for given fold
        '''
        # pylint: disable = too-many-arguments, too-many-positional-arguments
        log.debug(f'Plotting ROC curve for {ifold} fold')

        val_dir  = self._cfg['plotting']['val_dir']
        val_dir  = f'{val_dir}/fold_{ifold:03}'
        os.makedirs(val_dir, exist_ok=True)

        xval_ts, yval_ts, _ = roc_curve(l_lab_ts, l_prb_ts)
        xval_ts             = 1 - xval_ts
        area_ts             = auc(xval_ts, yval_ts)

        xval_tr, yval_tr, _ = roc_curve(l_lab_tr, l_prb_tr)
        xval_tr             = 1 - xval_tr
        area_tr             = auc(xval_tr, yval_tr)

        min_x = 0
        min_y = 0
        if 'min' in self._cfg['plotting']['roc']:
            [min_x, min_y] = self._cfg['plotting']['roc']['min']

        plt.plot(xval_ts, yval_ts, color='b', label=f'Test: {area_ts:.3f}')
        plt.plot(xval_tr, yval_tr, color='r', label=f'Train: {area_tr:.3f}')
        plt.xlabel('Signal efficiency')
        plt.ylabel('Background efficiency')
        plt.title(f'Fold: {ifold}')
        plt.xlim(min_x, 1)
        plt.ylim(min_y, 1)
        plt.legend()
        plt.savefig(f'{val_dir}/roc.png')
        plt.close()
    # ---------------------------------------------
    def _plot_features(self):
        '''
        Will plot the features, based on the settings in the config
        '''
        d_cfg = self._cfg['plotting']['features']
        ptr   = Plotter(d_rdf = {'Signal' : self._rdf_sig, 'Background' : self._rdf_bkg}, cfg=d_cfg)
        ptr.run()
    # ---------------------------------------------
    def run(self):
        '''
        Will do the training
        '''
        self._plot_features()

        l_mod = self._get_models()
        for ifold, mod in enumerate(l_mod):
            self._save_model(mod, ifold)
# ---------------------------------------------
