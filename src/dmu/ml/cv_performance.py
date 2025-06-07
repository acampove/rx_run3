'''
This module contains the class CVPerformance
'''
from ROOT                  import RDataFrame
from dmu.ml.cv_classifier  import CVClassifier
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:ml:cv_performance')
# -----------------------------------------------------
class CVPerformance:
    '''
    This class is meant to:

    - Compare the classifier performance, through the ROC curve, of a model, for a given background and signal sample
    '''
    # ---------------------------
    def load(
            self,
            name  : str,
            sig   : RDataFrame,
            bkg   : RDataFrame,
            model : list[CVClassifier] ) -> None:
        '''
        Method in charge of picking up model and data

        Parameters
        --------------------------
        name : Label of combination, used for plots
        sig  : ROOT dataframe storing signal samples
        bkg  : ROOT dataframe storing background samples
        model: List of instances of the CVClassifier
        '''
        log.info(f'Loading {name}')
    # ---------------------------
    def save(self, path : str) -> None:
        '''
        Directory path where outputs will be saved
        '''
# -----------------------------------------------------
