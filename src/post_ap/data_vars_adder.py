'''
Module containing DataVarsAdder class
'''

from ROOT import RDataFrame, Numba

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('post_ap:data_vars_adder')

# -------------------------------------
@Numba.Declare(['int', 'int'], 'int')
def get_block(run_number : int, fill_number : int) -> int:
    return 1
# -------------------------------------
class DataVarsAdder:
    '''
    Class used to add variables to dataframes that only make sense for data
    It adds:

    - block      : Block number 
    - is_good_run: Check for run data quality
    '''
    # -------------------------------------
    def __init__(self, rdf : RDataFrame):
        self._rdf = rdf
    # -------------------------------------
    def _add_dataq(self, rdf : RDataFrame) -> RDataFrame:
        log.info('Defining is_good_run')
        return rdf
    # -------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe with all variables added (or booked in this case)
        '''
        rdf = self._rdf
        rdf = rdf.Define('block', 'Numba::get_block(RUNNUMBER, FillNumber)')
        rdf = self._add_dataq(rdf)

        return rdf
# -------------------------------------
