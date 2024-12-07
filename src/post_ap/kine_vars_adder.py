'''
Module containing the KinematicsVarsAdder class
'''

from ROOT                    import RDataFrame
from dmu.logging.log_store   import LogStore

log = LogStore.add_logger('post_ap:kine_vars_adder')
# ------------------------------------------------------------------
class KinematicsVarsAdder:
    '''
    Class that adds kinematic variables to RDataFrame
    '''
    def __init__(self, rdf : RDataFrame, variables : list[str]):
        self._rdf              = rdf
        self._l_var            = variables
        self._l_not_a_particle = ['BUNCHCROSSING']
    # --------------------------------------------------
    def _get_particles(self) -> list[str]:
        v_name = self._rdf.GetColumnNames()
        l_name = [ name.c_str() for name in v_name ]
        l_name = [ name         for name in l_name if name.endswith('_ID') ]
        l_name = [ name.replace('_ID', '') for name in l_name ]
        l_name = [ name for name in l_name if name not in self._l_not_a_particle]

        log.info(f'Found particles: {l_name}')

        return l_name
    # --------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Will return dataframe with variables added
        '''
        l_part = self._get_particles()

        return self._rdf
# ------------------------------------------------------------------
