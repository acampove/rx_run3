'''
Module containing CVDiagnostics class
'''
import os

import numpy
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                  import RDataFrame
from dmu.ml.cv_classifier  import CVClassifier
from dmu.ml.cv_predict     import CVPredict
from dmu.logging.log_store import LogStore

NPA=numpy.ndarray

log = LogStore.add_logger('dmu:ml:cv_diagnostics')
# -------------------------
class CVDiagnostics:
    '''
    Class meant to rundiagnostics on classifier

    Correlations
    ------------------
    Will calculate correlations between features + signal probability and some external target variable specified in the config
    '''
    # -------------------------
    def __init__(self, models : list[CVClassifier], rdf : RDataFrame, cfg : dict):
        self._l_model = models
        self._cfg     = cfg
        self._rdf     = rdf
        self._target  = cfg['correlations']['target']
        self._l_feat  = self._get_features()
    # -------------------------
    def _get_features(self) -> list[str]:
        cfg   = self._l_model[0].cfg
        l_var = cfg['training']['features']

        return l_var
    # -------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        cfg    = self._l_model[0].cfg
        d_def  = cfg['dataset']['define']
        for var, expr in d_def.items():
            rdf = rdf.Define(var, expr)

        return rdf
    # -------------------------
    def _get_scores(self) -> NPA:
        if 'score_from_rdf' not in self._cfg:
            log.debug('Using score from model')
            prd = CVPredict(models=self._l_model, rdf = self._rdf)

            return prd.predict()

        name = self._cfg['score_from_rdf']
        log.debug(f'Picking up score from dataframe, column: {name}')
        arr_score = self._rdf.AsNumpy([name])[name]

        return arr_score
    # -------------------------
    def _get_arrays(self) -> dict[str, NPA]:
        rdf   = self._add_columns(self._rdf)
        l_col = [ name.c_str() for name in rdf.GetColumnNames() ]

        missing= False
        l_var  = self._l_feat + [self._target]
        for var in l_var:
            if var not in l_col:
                log.error(f'{"Missing":<20}{var}')
                missing=True

        if missing:
            raise ValueError('Columns missing')

        d_var          = rdf.AsNumpy(l_var)
        d_var['score'] = self._get_scores() 

        return d_var
    # -------------------------
    def _run_correlations(self, method : str):
        d_arr      = self._get_arrays()
        arr_target = d_arr[self._target]

        d_corr= {}
        for name, arr_val in d_arr.items():
            if name == self._target:
                continue

            d_corr[name] = self._calculate_correlations(var=arr_val, target=arr_target, method=method)

        self._plot_correlations(d_corr, method)
    # -------------------------
    def _plot_correlations(self, d_corr : dict[str,float], method : str) -> None:
        df = pnd.DataFrame.from_dict(d_corr, orient="index", columns=['Correlation'])
        df = df.sort_values(by='Correlation')

        figsize     = self._cfg['correlations']['figure']['size']
        df.plot(kind='bar', legend=False, figsize=figsize)

        out_dir = self._cfg['output'] + '/correlations'
        os.makedirs(out_dir, exist_ok=True)

        plot_path = f'{out_dir}/{method}.png'
        log.info(f'Saving to: {plot_path}')

        title = f'W.R.T. {self._target}'

        plt.ylim(-1, +1)
        plt.title(title)
        plt.ylabel('Correlation')
        plt.grid()
        plt.xticks(rotation=30)
        plt.tight_layout()
        plt.savefig(plot_path)
        plt.close()
    # -------------------------
    def _calculate_correlations(self, var : NPA, target : NPA, method : str) -> float:
        if method == 'pearson':
            mat = numpy.corrcoef(var, target)
            return mat[0,1]

        raise NotImplementedError(f'Correlation coefficient {method} not implemented')
    # -------------------------
    def run(self) -> None:
        '''
        Runs diagnostics
        '''
        if 'correlations' in self._cfg:
            for method in self._cfg['correlations']['methods']:
                self._run_correlations(method=method)
# -------------------------
