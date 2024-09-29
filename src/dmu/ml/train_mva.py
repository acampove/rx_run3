'''
Module with TrainMva class
'''
import os

import joblib
import pandas as pnd
import numpy
import matplotlib.pyplot as plt

from sklearn.metrics         import roc_curve, auc
from sklearn.model_selection import StratifiedKFold

from dmu.ml.cv_classifier    import CVClassifier as cls
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter    import Plotter

log = LogStore.add_logger('data_checks:train_mva')
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
        cfg (dict)          : Dictionary storing configuration for training
        '''
        self._rdf_bkg = bkg
        self._rdf_sig = sig
        self._cfg    = cfg if cfg is not None else {}

        self._l_ft_name = None
        self._l_model   = None
    # ---------------------------------------------
    def _get_features(self):
        '''
        Returns pandas dataframe with features
        '''

        d_ft_bkg = self._rdf_bkg.AsNumpy(self._l_ft_name)
        d_ft_sig = self._rdf_sig.AsNumpy(self._l_ft_name)

        df_bkg = pnd.DataFrame(d_ft_bkg)
        df_sig = pnd.DataFrame(d_ft_sig)

        df     = pnd.concat([df_bkg, df_sig], axis=0)

        log.info(f'Using features with shape: {df.shape}')

        return df
    # ---------------------------------------------
    def _get_labels(self):
        '''
        Returns labels, 0 for background, 1 for signal
        '''
        n_bkg  = self._rdf_bkg.Count().GetValue()
        n_sig  = self._rdf_sig.Count().GetValue()
        l_lab = n_bkg * [0] + n_sig * [1]

        arr_lab = numpy.array(l_lab)

        log.info(f'Using labels with shape: {arr_lab.shape}')

        return arr_lab
    # ---------------------------------------------
    def _get_model(self):
        # pylint: disable = too-many-locals
        '''
        Will create models, train them and return them
        '''
        df_ft = self._get_features()
        l_lab = self._get_labels()
        nfold = self._cfg['training']['nfold']
        rdmst = self._cfg['training']['rdm_stat']

        kfold = StratifiedKFold(n_splits=nfold, shuffle=True, random_state=rdmst)

        l_model=[]
        ifold=0
        for l_itr, l_its in kfold.split(df_ft, l_lab):
            model    = cls(cfg = self._cfg)
            df_ft_tr = df_ft.iloc[l_itr]
            l_lab_tr = l_lab[l_itr]

            log.debug(f'Training feature shape: {df_ft_tr.shape}')
            log.debug(f'Training label size: {len(l_lab_tr)}')

            model.fit(df_ft_tr, l_lab_tr)
            l_model.append(model)

            l_lab_prob_tr   = model.predict_proba(df_ft_tr, on_training_ok=True)
            l_lab_true_tr   = l_lab[l_itr]
            arr_sig_tr, arr_bkg_tr = self._split_scores(prob=l_lab_prob_tr, true=l_lab_true_tr)

            df_ft_ts         = df_ft.iloc[l_its]

            l_lab_prob_ts    = model.predict_proba(df_ft_ts)
            l_lab_true_ts    = l_lab[l_its]
            arr_sig_ts, arr_bkg_ts = self._split_scores(prob=l_lab_prob_ts, true=l_lab_true_ts)

            self._plot_scores(arr_sig_tr, arr_sig_ts, arr_bkg_tr, arr_bkg_ts, ifold)

            l_sig_prob_ts = [ prob[1] for prob in l_lab_prob_ts]
            l_sig_prob_tr = [ prob[1] for prob in l_lab_prob_tr]

            self._plot_roc(l_lab_true_ts, l_sig_prob_ts, l_lab_true_tr, l_sig_prob_tr, ifold)

            ifold+=1

        return l_model
    # ---------------------------------------------
    def _split_scores(self, prob=None, true=None):
        '''
        Will split the testing scores (predictions) based on the training scores

        tst is a list of lists as [p_bkg, p_sig]
        '''

        l_sig = [ prb[1] for prb, lab in zip(prob, true) if lab == 1]
        l_bkg = [ prb[1] for prb, lab in zip(prob, true) if lab == 0]

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
        # pylint: disable = too-many-arguments
        '''
        Will plot an array of scores, associated to a given fold
        '''
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
    def _plot_roc(self, l_lab_ts, l_prb_ts, l_lab_tr, l_prb_tr, ifold):
        '''
        Takes the labels and the probabilities and plots ROC
        curve for given fold
        '''
        # pylint: disable = too-many-arguments
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

        self._l_ft_name = self._cfg['training']['features']

        self._plot_features()

        l_mod = self._get_model()
        for ifold, mod in enumerate(l_mod):
            self._save_model(mod, ifold)
# ---------------------------------------------
