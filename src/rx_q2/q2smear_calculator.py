'''
Module containing Q2SmearCalculator class
'''

import numpy
import pandas as pnd
from ROOT                   import RDataFrame, RDF
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('rx_q2:q2smear_calculator')
# ------------------------------------
class WrongQ2SmearInput(Exception):
    '''
    Exception meant to be risen if wrong input is provided to Q2SmearCalculator
    '''
    def __init__(self, message : str):
        super().__init__(message)
# ------------------------------------
class Q2SmearCalculator:
    '''
    Class intended to operate only on simulated datasets from the electron channel to:

    - Read mass scales and resolutions
    - Calculated smeared masses
    - Return them in a dataframe
    '''
    # ------------------------------------
    def __init__(self, rdf : RDataFrame, mass_ee : str = 'Jpsi_M_brem_track_2'):
        '''
        rdf : ROOT Dataframe with the data
        mass_ee : Name of branch with masses to smear
        '''
        self._validate_input(rdf)

        self._mass_ee_pdg = 3096.9
        log.debug(f'Using Jpsi PDG mass: {self._mass_ee_pdg:.2f}')

        self._rdf     = rdf
        self._l_var   = [mass_ee, 'nbrem', 'block', 'EVENTNUMBER', 'RUNNUMBER'] # EVENTNUMBER and RUNNUMBER are needed to align samples
        self._mass_ee = mass_ee
    # ------------------------------------
    def _validate_input(self, rdf :  RDataFrame) -> None:
        l_col = [ name.c_str() for name in rdf.GetColumnNames() ]
        if 'L1_TRUEID' not in l_col:
            raise WrongQ2SmearInput('Dataframe does not have true information, e.g. it is not MC')

        rdf_small  = rdf.Range(10)
        arr_trueid = rdf_small.AsNumpy(['L1_TRUEID'])['L1_TRUEID']
        arr_trueid = numpy.abs(arr_trueid)

        if not numpy.all(arr_trueid == 11):
            raise WrongQ2SmearInput('Input does not belong to electron channel')
    # ------------------------------------
    def _calculate_smeared_mass(self, row : pnd.Series) -> float:
        factor = numpy.random.normal(loc=0, scale=1000_000)

        return factor + row[self._mass_ee]
    # ------------------------------------
    def get_rdf(self, name : str = None) -> RDataFrame:
        '''
        Will return ROOT dataframe with smeared mass of dielectron 

        name: Name of branch containing smeared mass
        '''
        if name is None:
            name = f'{self._mass_ee}_smr'

        log.info(f'Storing smeared di-electron mass in: {name}')

        data     = self._rdf.AsNumpy(self._l_var)
        df       = pnd.DataFrame(data)
        df[name] = df.apply(self._calculate_smeared_mass, axis=1)
        rdf      = RDF.FromPandas(df)

        return rdf
# ------------------------------------
