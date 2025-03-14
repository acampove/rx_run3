'''
Module containing CVDiagnostics class
'''
from ROOT                  import RDataFrame
from dmu.ml.cv_classifier  import CVClassifier
from dmu.ml.cv_predict     import CVPredict

# -------------------------
class CVDiagnostics:
    '''
    Class meant to rundiagnostics on classifier
    '''
    # -------------------------
    def __init__(self, models : list[CVClassifier], rdf : RDataFrame, cfg : dict):
        self._l_model = models
        self._rdf     = rdf
        self._cfg     = cfg
    # -------------------------
    def run(self) -> None:
        '''
        Runs diagnostics
        '''
        pass
# -------------------------
