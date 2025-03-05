'''
Module with ElectronBiasCorrector class
'''
import pandas as pnd
from dmu.logging.log_store  import LogStore
from vector                 import MomentumObject3D as v3d
from vector                 import MomentumObject4D as v4d

from rx_data.brem_bias_corrector import BremBiasCorrector

log=LogStore.add_logger('rx_data:electron_bias_corrector')
# ---------------------------------
class ElectronBiasCorrector:
    '''
    Class meant to correct electron kinematics
    '''
    # ---------------------------------
    def __init__(self, skip_correction : bool = False):
        self._skip_correction = skip_correction
        self._mass            = 0.511
        self._bcor            = BremBiasCorrector()
        self._name : str

        if self._skip_correction:
            log.warning('Not applying electron bias correction')
        else:
            log.info('Applying electron bias correction')
    # ---------------------------------
    def _get_electron(self, row : pnd.Series, kind : str) -> v4d:
        px = self._attr_from_row(row, f'{self._name}_{kind}PX')
        py = self._attr_from_row(row, f'{self._name}_{kind}PY')
        pz = self._attr_from_row(row, f'{self._name}_{kind}PZ')

        e_3d = v3d(px=px, py=py, pz=pz)
        pt   = e_3d.pt
        eta  = e_3d.eta
        phi  = e_3d.phi

        e_4d = v4d(pt=pt, eta=eta, phi=phi, mass=self._mass)

        return e_4d
    # ---------------------------------
    def _get_ebrem(self, row : pnd.Series, e_track : v4d) -> v4d:
        e_full = self._get_electron(row, kind='')
        e_brem = e_full - e_track

        return e_brem
    # ---------------------------------
    def _correct_brem(self, e_brem : v4d, row : pnd.Series) -> v4d:
        if self._skip_correction:
            return e_brem

        brem_row = self._attr_from_row(row, f'{self._name}_BREMHYPOCOL')
        brem_col = self._attr_from_row(row, f'{self._name}_BREMHYPOROW')
        e_brem   = self._bcor.correct(brem=e_brem, row=brem_row, col=brem_col)

        return e_brem
    # ---------------------------------
    def _update_row(self, row : pnd.Series, e_corr : v4d) -> pnd.Series:
        l_var      = [
                f'{self._name}_PX',
                f'{self._name}_PY',
                f'{self._name}_PZ']

        row.loc[l_var] = [e_corr.px, e_corr.py, e_corr.pz]

        l_var      = [
                f'{self._name}_PT' ,
                f'{self._name}_ETA',
                f'{self._name}_PHI']

        row.loc[l_var] = [e_corr.pt, e_corr.eta, e_corr.phi]

        return row
    # ---------------------------------
    def _attr_from_row(self, row : pnd.Series, name : str) -> float:
        if hasattr(row, name):
            return getattr(row, name)

        log.error(f'Cannot find attribute {name} among:')
        for col_name in row.index:
            log.info(col_name)

        raise ValueError
    # ---------------------------------
    def correct(self, row : pnd.Series, name : str) -> pnd.Series:
        '''
        Corrects kinematics and returns row
        row  : Pandas dataframe row
        name : Particle name, e.g. L1
        '''
        self._name = name

        if not self._attr_from_row(row, f'{name}_HASBREMADDED'):
            return row

        e_track = self._get_electron(row, kind='TRACK_')
        e_brem  = self._get_ebrem(row, e_track)
        e_brem  = self._correct_brem(e_brem, row)
        e_corr  = e_track + e_brem
        row     = self._update_row(row, e_corr)

        return row
# ---------------------------------
