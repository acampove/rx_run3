'''
Module with ElectronBiasCorrector class
'''
import math
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
            log.warning('Skipping electron correction')
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
        e_4d = e_4d.to_pxpypzenergy()

        return e_4d
    # ---------------------------------
    def _get_ebrem(self, row : pnd.Series, e_track : v4d) -> v4d:
        e_full = self._get_electron(row, kind='')
        e_brem = e_full - e_track
        e_brem = e_brem.to_pxpypzenergy()

        self._check_massless_brem(e_brem)

        return e_brem
    # ---------------------------------
    def _check_massless_brem(self, e_brem : v4d) -> None:
        energy  = e_brem.e
        momentum= e_brem.p

        if not math.isclose(energy, momentum, rel_tol=1e-5):
            log.warning('Brem energy and momentum are not equal')
            log.info(f'{energy:.5f}=={momentum:.5f}')
        else:
            log.debug('Brem photon energy and momentum are close enough:')
            log.debug(f'{energy:.5f}=={momentum:.5f}')

        return e_brem
    # ---------------------------------
    def _correct_brem(self, e_brem : v4d, row : pnd.Series) -> v4d:
        if self._skip_correction:
            log.warning('Skipping electron correction')
            return e_brem

        log.info('Applying ecalo_bias correction')

        brem_row = self._attr_from_row(row, f'{self._name}_BREMHYPOROW')
        brem_col = self._attr_from_row(row, f'{self._name}_BREMHYPOCOL')
        brem_area= self._attr_from_row(row, f'{self._name}_BREMHYPOAREA')

        e_brem_corr = self._bcor.correct(brem=e_brem, row=brem_row, col=brem_col, area=brem_area)

        if e_brem_corr.isclose(e_brem, rtol=1e-5):
            momentum = e_brem.p
            log.warning(f'Correction did not change photon at row/column/region/momentum: {brem_row}/{brem_col}/{brem_area}/{momentum:.0f}')
            log.info(e_brem)
            log.info('--->')
            log.info(e_brem_corr)
        else:
            log.debug('Brem was corrected:')
            log.debug(e_brem)
            log.debug('--->')
            log.debug(e_brem_corr)

        self._check_massless_brem(e_brem_corr)

        return e_brem_corr
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

        for col_name in row.index:
            log.info(col_name)

        raise ValueError(f'Cannot find attribute {name} among:')
    # ---------------------------------
    def _correct_with_track_brem(self, e_track : v4d, e_brem : v4d, row : pnd.Series) -> v4d:
        if self._skip_correction:
            return e_track + e_brem

        log.info('Applying brem_track electron correction')
        brem_energy = self._attr_from_row(row, f'{self._name}_BREMTRACKBASEDENERGY')
        if brem_energy == 0:
            return e_track

        eta = e_track.eta
        phi = e_track.phi

        gamma  = v4d(pt=1, eta=eta, phi=phi, m=0)
        factor = brem_energy / gamma.e

        px     = factor * gamma.px
        py     = factor * gamma.py
        pz     = factor * gamma.pz
        e      = factor * gamma.e

        gamma  = v4d(px=px, py=py, pz=pz, e=e)

        self._check_massless_brem(gamma)

        return e_track + gamma
    # ---------------------------------
    def correct(self, row : pnd.Series, name : str, kind : str = 'brem_track') -> pnd.Series:
        '''
        Corrects kinematics and returns row
        row  : Pandas dataframe row
        name : Particle name, e.g. L1
        kind : Type of correction, [ecalo_bias, brem_track]
        '''
        self._name = name
        e_track = self._get_electron(row, kind='TRACK_')
        e_brem  = self._get_ebrem(row, e_track)

        if   kind == 'ecalo_bias':
            if not self._attr_from_row(row, f'{name}_HASBREMADDED'):
                return row

            e_brem  = self._correct_brem(e_brem, row)
            e_corr  = e_track + e_brem
        elif kind == 'brem_track':
            e_corr  = self._correct_with_track_brem(e_track, e_brem, row)
        else:
            raise NotImplementedError(f'Invalid correction of type: {kind}')

        row = self._update_row(row, e_corr)

        return row
# ---------------------------------
