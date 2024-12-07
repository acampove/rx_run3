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
    def __init__(self, rdf : RDataFrame, variables : dict[str,str]):
        self._rdf    = rdf
        self._d_expr = variables

        self._l_not_a_particle            = ['BUNCHCROSSING']
        self._l_branch    : list[str]
        self._l_var_added : list[str]     = []
    # --------------------------------------------------
    @property
    def names(self) -> list[str]:
        '''
        Returns list of names of variables added
        '''

        return self._l_var_added
    # --------------------------------------------------
    def _get_particles(self) -> list[str]:
        l_name = [ name         for name in self._l_branch if name.endswith('_ID') ]
        l_name = [ name.replace('_ID', '') for name in l_name ]
        l_name = [ name for name in l_name if name not in self._l_not_a_particle]

        log.info(f'Found particles: {l_name}')

        return l_name
    # --------------------------------------------------
    def _get_branch_names(self) -> list[str]:
        v_name = self._rdf.GetColumnNames()
        l_name = [ name.c_str() for name in v_name ]

        return l_name
    # --------------------------------------------------
    def _add_particle_variables(self, particle : str) -> None:
        for variable, expr in self._d_expr.items():
            expr = expr.replace('PARTICLE', particle)
            name = f'{particle}_{variable}'

            log.debug(f'{name:<15}{expr:<100}')
            if name in self._l_branch:
                self._rdf = self._rdf.Redefine(name, expr)
            else:
                self._rdf = self._rdf.Define(name, expr)

            self._l_var_added.append(name)
    # --------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Will return dataframe with variables added
        '''
        self._l_branch = self._get_branch_names()
        l_part         = self._get_particles()

        log.info('Adding kinematic variables')
        for part in l_part:
            self._add_particle_variables(part)

        return self._rdf
# ------------------------------------------------------------------
