'''
Module with MisCalculator class
'''

import fnmatch

from ROOT import RDF # type: ignore
from dmu.logging.log_store  import LogStore
from rx_common              import info

log = LogStore.add_logger('rx_data:mis_calculator')
# ----------------------------------------------
class MisCalculator:
    '''
    Class used to add missing variables to ROOT dataframes
    '''
    # -------------
    def __init__(self, rdf : RDF.RNode, trigger : str):
        '''
        Initializer taking dataframe and trigger, the latter is needed to know mass hypotheses of leptons
        '''
        self._rdf    = rdf
        self._trigger= trigger

        self._emass = 0.511 # electron mass
        self._mmass = 105.6 # muon mass
        self._kmass = 493.6 # mass of kaon

        self._lmass = self._get_lepton_mass(trigger)
    # -------------
    def _get_lepton_mass(self, trigger : str) -> float:
        if fnmatch.fnmatch(trigger, 'Hlt2RD_*EE*MVA*'):
            log.debug('Using electron mass hypothesis')
            return self._emass

        if fnmatch.fnmatch(trigger, 'Hlt2RD_*MuMu*MVA*'):
            log.debug('Using muon mass hypothesis')
            return self._mmass

        raise NotImplementedError(f'Cannot recognize trigger {trigger} as electron or muon')
    # -------------
    def _add_energy(self, rdf : RDF.RNode) -> RDF.RNode:
        l_col = [ name for name in rdf.GetColumnNames() ]

        if 'L1_PE' not in l_col:
            name = 'L1_P'
            rdf  = rdf.Define(f'{name}E', f'TMath::Sqrt({name}X * {name}X + {name}Y * {name}Y + {name}Z * {name}Z + {self._lmass} * {self._lmass})')

        if 'L2_PE' not in l_col:
            name = 'L2_P'
            rdf  = rdf.Define(f'{name}E', f'TMath::Sqrt({name}X * {name}X + {name}Y * {name}Y + {name}Z * {name}Z + {self._lmass} * {self._lmass})')

        project = info.project_from_trigger(trigger=self._trigger, lower_case=True)

        if project == 'rk' and 'H_PE' not in l_col:
            name = 'H_P'
            rdf  = rdf.Define(f'{name}E', f'TMath::Sqrt({name}X * {name}X + {name}Y * {name}Y + {name}Z * {name}Z + {self._kmass} * {self._kmass})')

        if project == 'rkst' and 'H1_PE' not in l_col:
            name = 'H1_P'
            rdf  = rdf.Define(f'{name}E', f'TMath::Sqrt({name}X * {name}X + {name}Y * {name}Y + {name}Z * {name}Z + {self._kmass} * {self._kmass})')

        if project == 'rkst' and 'H2_PE' not in l_col:
            name = 'H2_P'
            rdf  = rdf.Define(f'{name}E', f'TMath::Sqrt({name}X * {name}X + {name}Y * {name}Y + {name}Z * {name}Z + {self._kmass} * {self._kmass})')

        return rdf
    # -------------------------------
    def get_rdf(self) -> RDF.RNode:
        '''
        Returns dataframe after adding variables
        '''
        rdf = self._add_energy(self._rdf)

        return rdf
# ----------------------------------------------
