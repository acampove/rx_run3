'''
Module containing Q2SmearCalculator class
'''
import os
import numpy
import pandas as pnd
from ROOT                           import RDataFrame, RDF
from dmu.logging.log_store          import LogStore
from dmu.generic.version_management import get_last_version

log = LogStore.add_logger('rx_q2:q2smear_calculator')
# ------------------------------------
class Q2SmearCorrector:
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
        self._df      = self._get_scales()
    # ------------------------------------
    def _get_scales(self) -> pnd.DataFrame:
        ana_dir = os.environ['ANADIR']
        par_dir = get_last_version(f'{ana_dir}/q2/fits', version_only=False)
        par_path= f'{par_dir}/parameters.json'
        log.info(f'Reading scales from: {par_path}')
        df      = pnd.read_json(par_path)

        return df
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
    def _read_quantity(self, row : pnd.Series, kind : str) -> float:
        brem = row['nbrem']
        block= row['block']

        df   = self._df
        if kind == 'mu_mc':
            mu_mc = df.loc[ (df['block'] == block) & (df['brem'] == brem) & (df['sample'] == 'sim'), 'mu_val' ].iloc[0]
            return mu_mc

        if kind == 'reso':
            sg_dt = df.loc[ (df['block'] == block) & (df['brem'] == brem) & (df['sample'] == 'dat'), 'sg_val' ].iloc[0]
            sg_mc = df.loc[ (df['block'] == block) & (df['brem'] == brem) & (df['sample'] == 'sim'), 'sg_val' ].iloc[0]
            return sg_dt / sg_mc

        if kind == 'scale':
            mu_dt = df.loc[ (df['block'] == block) & (df['brem'] == brem) & (df['sample'] == 'dat'), 'mu_val' ].iloc[0]
            mu_mc = df.loc[ (df['block'] == block) & (df['brem'] == brem) & (df['sample'] == 'sim'), 'mu_val' ].iloc[0]

            return mu_dt - mu_mc

        raise NotImplementedError(f'Invalid quantity: {kind}')
    # ------------------------------------
    def _calculate_smeared_mass(self, row : pnd.Series) -> float:
        jpsi_mass_reco = row[self._mass_ee]
        jpsi_mass_true = self._mass_ee_pdg

        mu_mc = self._read_quantity(row, kind='mu_mc')
        reso  = self._read_quantity(row, kind= 'reso')
        scale = self._read_quantity(row, kind='scale')
        mass  = jpsi_mass_true + reso * (jpsi_mass_reco - jpsi_mass_true) + scale + (1 - reso) * (mu_mc - self._mass_ee_pdg)

        return mass
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
