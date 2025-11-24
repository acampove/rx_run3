'''
Module with Stats class
'''
from ROOT      import RDF # type: ignore
from dmu       import LogStore
from rx_common import Trigger

from rx_data.spec_maker import SpecMaker

log=LogStore.add_logger('rx_data:stats')
# ----------------------------------------
class Stats:
    '''
    Class meant to provide number of candidates
    '''
    # ----------------------------------------
    def __init__(self, sample : str, trigger : Trigger):
        '''
        sample  (str): MC sample identifier
        trigger (str): HLT2 trigger
        '''
        self._sample  = sample
        self._trigger = trigger
    # ----------------------------------------
    def get_entries(self, tree : str) -> int:
        '''
        Takes tree name, returns number of entries
        '''
        mkr = SpecMaker(
            sample  = self._sample, 
            trigger = self._trigger,
            tree    = tree)
        json_path = mkr.get_spec_path(per_file=False)

        rdf    = RDF.Experimental.FromSpec(str(json_path))
        val    = rdf.Count().GetValue()

        return val
# ----------------------------------------
