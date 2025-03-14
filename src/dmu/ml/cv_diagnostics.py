'''
Module containing CVDiagnostics class
'''
import numpy
import pandas as pnd

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
    '''
    # -------------------------
    def __init__(self, models : list[CVClassifier], rdf : RDataFrame, cfg : dict):
        self._l_model = models
        self._d_var   = self._get_variables()
        self._rdf     = rdf
        self._cfg     = cfg
    # -------------------------
    def _get_variables(self) -> list[str]:
        cfg    = self._l_model[0].cfg
        l_feat = cfg['training']['features']

        return l_feat
    # -------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        cfg    = self._l_model[0].cfg
        d_def  = cfg['dataset']['define']
        for var, expr in d_def.items():
            rdf = rdf.Define(var, expr)

        return rdf
    # -------------------------
    def _get_arrays(self) -> dict[str, NPA]:
        rdf   = self._add_columns(self._rdf)
        l_var = list(self._d_var)
        l_col = [ name.c_str() for name in rdf.GetColumnNames() ]

        missing=False
        for var in l_var:
            if var not in l_col:
                log.error(f'{"Missing":<20}{var}')
                missing=True

        if missing:
            raise ValueError('Columns missing')

        d_var = rdf.AsNumpy(l_var)

        return d_var
    # -------------------------
    def _run_correlations(self, method : str):
        prd      = CVPredict(models=self._l_model, rdf = self._rdf)
        arr_prob = prd.predict()

        d_arr = self._get_arrays()
        d_corr= {}
        for name, arr_val in d_arr.items():
            d_corr[name] = self._calculate_correlations(var=arr_val, prob=arr_prob, method=method)

        self._plot_correlations(d_corr)
    # -------------------------
    def _plot_correlations(self, d_corr : dict[str,float]) -> None:
        df = pnd.DataFrame.from_dict(d_corr, orient="index", columns=['Correlation'])

        print(df)
    # -------------------------
    def _calculate_correlations(self, var : NPA, prob : NPA, method : str) -> float:
        if method == 'pearson':
            mat = numpy.corrcoef(var, prob)
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
