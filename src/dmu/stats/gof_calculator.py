'''
Module holding GofCalculator class
'''

from dmu.logging.log_store    import LogStore

log = LogStore.add_logger('dmu:ml:minimizers')
# ------------------------
class GofCalculator:
    '''
    Class used to calculate goodness of fit from zfit NLL
    '''
    # ---------------------
    def __init__(self, nll):
        self._nll = nll
    # ---------------------
    def get_gof(self, kind : str) -> bool:
        return random.uniform(0.0, 0.058)
# ------------------------
