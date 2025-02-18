'''
Module with TrainMva class
'''
# pylint: disable = too-many-locals, no-name-in-module
# pylint: disable = too-many-arguments, too-many-positional-arguments

import os

import joblib
import pandas as pnd
import numpy
import matplotlib.pyplot as plt

from sklearn.metrics         import roc_curve, auc
from sklearn.model_selection import StratifiedKFold

from ROOT import RDataFrame, RDF

import dmu.ml.utilities         as ut
import dmu.pdataframe.utilities as put
import dmu.plotting.utilities   as plu

from dmu.ml.cv_classifier    import CVClassifier as cls
from dmu.plotting.plotter_1d import Plotter1D    as Plotter
from dmu.plotting.matrix     import MatrixPlotter
from dmu.logging.log_store   import LogStore

npa = numpy.ndarray
log = LogStore.add_logger('dmu:ml:train_mva')
# ---------------------------------------------
class TrainMva:
    '''
    Interface to scikit learn used to train classifier
    '''
    # ---------------------------------------------
    def __init__(self, bkg : RDataFrame, sig : RDataFrame, cfg : dict):
        '''
        bkg (ROOT dataframe): Holds real data
        sig (ROOT dataframe): Holds simulation
        cfg (dict)          : Dictionary storing configuration for training
        '''
        self._cfg       = cfg
        self._l_ft_name = self._cfg['training']['features']

        df_ft_sig, l_lab_sig = self._get_sample_inputs(rdf = sig, label = 1)
        df_ft_bkg, l_lab_bkg = self._get_sample_inputs(rdf = bkg, label = 0)

        self._df_ft = pnd.concat([df_ft_sig, df_ft_bkg], axis=0)
        self._l_lab = numpy.array(l_lab_sig + l_lab_bkg)

        self._rdf_bkg = self._get_rdf(rdf = bkg, df=df_ft_bkg)
        self._rdf_sig = self._get_rdf(rdf = sig, df=df_ft_sig)
    # ---------------------------------------------
    def _get_rdf(self, rdf : RDataFrame, df : pnd.DataFrame) -> RDataFrame:
        '''
        Takes original ROOT dataframe and pre-processed features dataframe
        Adds missing branches to latter and returns expanded ROOT dataframe
        '''

        l_pnd_col = df.columns.tolist()
        l_rdf_col = [ name.c_str() for name in rdf.GetColumnNames() ]
        l_mis_col = [ col for col in l_rdf_col if col not in l_pnd_col ]

        log.debug(f'Adding extra-nonfeature columns: {l_mis_col}')

        d_data = rdf.AsNumpy(l_mis_col)
        df_ext = pnd.DataFrame(d_data)
        df_all = pnd.concat([df, df_ext], axis=1)

        return RDF.FromPandas(df_all)
    # ---------------------------------------------
    def _pre_process_nans(self, df : pnd.DataFrame) -> pnd.DataFrame:
        if 'dataset' not in self._cfg:
            return df

        if 'nan' not in self._cfg['dataset']:
            log.debug('dataset/nan section not found, not pre-processing NaNs')
            return df

        d_name_val = self._cfg['dataset']['nan']
        log.info(70 * '-')
        log.info('Doing NaN replacements')
        log.info(70 * '-')
        for var, val in d_name_val.items():
            nna = df[var].isna().sum()

            log.info(f'{var:<20}{"--->":20}{val:<20.3f}{nna}')
            df[var] = df[var].fillna(val)
        log.info(70 * '-')

        return df
    # ---------------------------------------------
    def _get_sample_inputs(self, rdf : RDataFrame, label : int) -> tuple[pnd.DataFrame, list[int]]:
        d_ft = rdf.AsNumpy(self._l_ft_name)
        df   = pnd.DataFrame(d_ft)
        df   = self._pre_process_nans(df)
        df   = ut.cleanup(df)
        l_lab= len(df) * [label]

        return df, l_lab
    # ---------------------------------------------
    def _get_model(self, arr_index : npa) -> cls:
        model = cls(cfg = self._cfg)
        df_ft = self._df_ft.iloc[arr_index]
        l_lab = self._l_lab[arr_index]

        log.debug(f'Training feature shape: {df_ft.shape}')
        log.debug(f'Training label size: {len(l_lab)}')

        model.fit(df_ft, l_lab)

        return model
    # ---------------------------------------------
    def _get_models(self):
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

            self._save_feature_importance(model, ifold)
            self._plot_correlation(arr_itr, ifold)
            self._plot_scores(arr_sig_sig_tr, arr_sig_sig_ts, arr_sig_bkg_tr, arr_sig_bkg_ts, ifold)
            self._plot_roc(arr_lab_ts, arr_sig_all_ts, arr_lab_tr, arr_sig_all_tr, ifold)

            ifold+=1

        return l_model
    # ---------------------------------------------
    def _labels_from_varnames(self, l_var_name : list[str]) -> list[str]:
        try:
            d_plot = self._cfg['plotting']['features']['plots']
        except ValueError:
            log.warning('Cannot find plotting/features/plots section in config, using dataframe names')
            return l_var_name

        l_label = []
        for var_name in l_var_name:
            if var_name not in d_plot:
                log.warning(f'No plot found for: {var_name}')
                l_label.append(var_name)
                continue

            d_setting = d_plot[var_name]
            [xlab, _ ]= d_setting['labels']

            l_label.append(xlab)

        return l_label
    # ---------------------------------------------
    def _save_feature_importance(self, model : cls, ifold : int) -> None:
        l_var_name           = self._df_ft.columns.tolist()

        d_data               = {}
        d_data['Variable'  ] = self._labels_from_varnames(l_var_name)
        d_data['Importance'] = 100 * model.feature_importances_

        val_dir  = self._cfg['plotting']['val_dir']
        val_dir  = f'{val_dir}/fold_{ifold:03}'
        os.makedirs(val_dir, exist_ok=True)

        df = pnd.DataFrame(d_data)
        df = df.sort_values(by='Importance', ascending=False)

        table_path = f'{val_dir}/importance.tex'
        d_form = {'Variable' : '{}', 'Importance' : '{:.1f}'}
        put.df_to_tex(df, table_path, d_format = d_form)
    # ---------------------------------------------
    def _get_scores(self, model : cls, arr_index : npa, on_training_ok : bool) -> tuple[npa, npa, npa, npa]:
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
    def _split_scores(self, arr_prob : npa, arr_label : npa) -> tuple[npa, npa]:
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
    def _save_model(self, model : cls, ifold : int) -> None:
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
    def _get_correlation_cfg(self, df : pnd.DataFrame, ifold : int) -> dict:
        l_var_name = df.columns.tolist()
        l_label    = self._labels_from_varnames(l_var_name)
        cfg = {
                'labels'     : l_label,
                'title'      : f'Fold {ifold}',
                'label_angle': 45,
                'upper'      : True,
                'zrange'     : [-1, +1],
                'size'       : [7, 7],
                'format'     : '{:.3f}',
                'fontsize'   : 12,
                }

        if 'correlation' not in self._cfg['plotting']:
            log.info('Using default correlation plotting configuration')
            return cfg

        log.debug('Updating correlation plotting configuration')
        custom = self._cfg['plotting']['correlation']
        cfg.update(custom)

        return cfg
    # ---------------------------------------------
    def _plot_correlation(self, arr_index : npa, ifold : int) -> None:
        df_ft = self._df_ft.iloc[arr_index]
        cfg = self._get_correlation_cfg(df_ft, ifold)
        cov = df_ft.corr()
        mat = cov.to_numpy()

        log.debug(f'Plotting correlation for {ifold} fold')

        val_dir  = self._cfg['plotting']['val_dir']
        val_dir  = f'{val_dir}/fold_{ifold:03}'
        os.makedirs(val_dir, exist_ok=True)

        obj = MatrixPlotter(mat=mat, cfg=cfg)
        obj.plot()
        plt.savefig(f'{val_dir}/covariance.png')
        plt.close()
    # ---------------------------------------------
    def _get_nentries(self, arr_val : npa) -> str:
        size = len(arr_val)
        size = size / 1000.

        return f'{size:.2f}K'
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

        plt.hist(arr_sig_trn, alpha   =   0.3, bins=50, range=(0,1), color='b', density=True, label='Signal Train: '    + self._get_nentries(arr_sig_trn))
        plt.hist(arr_sig_tst, histtype='step', bins=50, range=(0,1), color='b', density=True, label='Signal Test: '     + self._get_nentries(arr_sig_tst))

        plt.hist(arr_bkg_trn, alpha   =   0.3, bins=50, range=(0,1), color='r', density=True, label='Background Train: '+ self._get_nentries(arr_bkg_trn))
        plt.hist(arr_bkg_tst, histtype='step', bins=50, range=(0,1), color='r', density=True, label='Background Test: ' + self._get_nentries(arr_bkg_tst))

        plt.legend()
        plt.title(f'Fold: {ifold}')
        plt.xlabel('Signal probability')
        plt.ylabel('Normalized')
        plt.savefig(f'{val_dir}/scores.png')
        plt.close()
    # ---------------------------------------------
    def _plot_roc(self,
                  l_lab_ts : npa,
                  l_prb_ts : npa,
                  l_lab_tr : npa,
                  l_prb_tr : npa,
                  ifold    : int):
        '''
        Takes the labels and the probabilities and plots ROC
        curve for given fold
        '''
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

        max_x = 1
        max_y = 1
        if 'max' in self._cfg['plotting']['roc']:
            [max_x, max_y] = self._cfg['plotting']['roc']['max']

        plt.plot(xval_ts, yval_ts, color='b', label=f'Test: {area_ts:.3f}')
        plt.plot(xval_tr, yval_tr, color='r', label=f'Train: {area_tr:.3f}')
        self._plot_probabilities(xval_ts, yval_ts, l_prb_ts, l_lab_ts)

        plt.xlabel('Signal efficiency')
        plt.ylabel('Background rejection')
        plt.title(f'Fold: {ifold}')
        plt.xlim(min_x, max_x)
        plt.ylim(min_y, max_y)
        plt.grid()
        plt.legend()
        plt.savefig(f'{val_dir}/roc.png')
        plt.close()
    # ---------------------------------------------
    def _plot_probabilities(self,
                            arr_seff: npa,
                            arr_brej: npa,
                            arr_sprb: npa,
                            arr_labl: npa) -> None:

        roc_cfg = self._cfg['plotting']['roc']
        if 'annotate' not in roc_cfg:
            log.debug('Annotation section in the ROC curve config not found, skipping annotation')
            return

        l_sprb   = [ sprb for sprb, labl in zip(arr_sprb, arr_labl) if labl == 1 ]
        arr_sprb = numpy.array(l_sprb)

        plt_cfg = roc_cfg['annotate']
        if 'sig_eff' not in plt_cfg:
            l_seff_target = [0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95]
        else:
            l_seff_target = plt_cfg['sig_eff']
            del plt_cfg['sig_eff']

        arr_seff_target = numpy.array(l_seff_target)
        arr_quantile    = 1 - arr_seff_target

        l_score = numpy.quantile(arr_sprb, arr_quantile)
        l_seff  = []
        l_brej  = []

        log.debug(60 * '-')
        log.debug(f'{"SigEff":20}{"BkgRej":20}{"Score":20}')
        log.debug(60 * '-')
        for seff_target, score in zip(arr_seff_target, l_score):
            arr_diff = numpy.abs(arr_seff - seff_target)
            ind      = numpy.argmin(arr_diff)

            seff     = arr_seff[ind]
            brej     = arr_brej[ind]

            log.debug(f'{seff:<20.3f}{brej:<20.3f}{score:<20.2f}')

            l_seff.append(seff)
            l_brej.append(brej)

        plu.annotate(l_x=l_seff, l_y=l_brej, l_v=l_score, **plt_cfg)
    # ---------------------------------------------
    def _plot_features(self):
        '''
        Will plot the features, based on the settings in the config
        '''
        d_cfg = self._cfg['plotting']['features']
        ptr   = Plotter(d_rdf = {'Signal' : self._rdf_sig, 'Background' : self._rdf_bkg}, cfg=d_cfg)
        ptr.run()
    # ---------------------------------------------
    def _save_settings_to_tex(self) -> None:
        self._save_nan_conversion()
        self._save_hyperparameters_to_tex()
    # ---------------------------------------------
    def _save_nan_conversion(self) -> None:
        if 'dataset' not in self._cfg:
            return

        if 'nan' not in self._cfg['dataset']:
            log.debug('NaN section not found, not saving it')
            return

        d_nan = self._cfg['dataset']['nan']
        l_var = list(d_nan)
        l_lab = self._labels_from_varnames(l_var)
        l_val = list(d_nan.values())

        d_tex = {'Variable' : l_lab, 'Replacement' : l_val}
        df    = pnd.DataFrame(d_tex)
        val_dir  = self._cfg['plotting']['val_dir']
        os.makedirs(val_dir, exist_ok=True)
        put.df_to_tex(df, f'{val_dir}/nan_replacement.tex')
    # ---------------------------------------------
    def _save_hyperparameters_to_tex(self) -> None:
        if 'hyper' not in self._cfg['training']:
            raise ValueError('Cannot find hyper parameters in configuration')

        d_hyper = self._cfg['training']['hyper']
        d_form  = { f'\\verb|{key}|' : f'\\verb|{val}|' for key, val in d_hyper.items() }
        d_latex = { 'Hyperparameter' : list(d_form.keys()), 'Value' : list(d_form.values())}

        df = pnd.DataFrame(d_latex)
        val_dir  = self._cfg['plotting']['val_dir']
        os.makedirs(val_dir, exist_ok=True)
        put.df_to_tex(df, f'{val_dir}/hyperparameters.tex')
    # ---------------------------------------------
    def run(self, skip_fit : bool = False) -> None:
        '''
        Will do the training

        skip_fit: By default false, if True, it will only do the plots of features and save tables
        '''
        self._save_settings_to_tex()
        self._plot_features()

        if skip_fit:
            return

        l_mod = self._get_models()
        for ifold, mod in enumerate(l_mod):
            self._save_model(mod, ifold)
# ---------------------------------------------
