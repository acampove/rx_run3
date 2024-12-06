'''
Module containing DataVarsAdder class
'''

from ROOT import RDataFrame

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('post_ap:data_vars_adder')
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
    def _add_block(self, rdf : RDataFrame) -> RDataFrame:
        return rdf
    # -------------------------------------
    def _add_dataq(self, rdf : RDataFrame) -> RDataFrame:
        return rdf
    # -------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns dataframe with all variables added (or booked in this case)
        '''
        rdf = self._rdf
        rdf = self._add_block(rdf)
        rdf = self._add_dataq(rdf)

        return rdf
# -------------------------------------
