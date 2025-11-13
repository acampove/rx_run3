'''
Module holding FitSummary class
'''
import os
from pathlib import Path

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:fit_summary')
# -------------------------------
class FitSummary:
    '''
    Class meant to: 

    - Collect yields an brem fractions from fits
    - Plot yields and fractions
    - Store information in JSON for further usage
    '''
    # ----------------------
    def __init__(self, name : str) -> None:
        '''
        Parameters
        -------------
        name : Name of directory in {ANADIR}/fits/data/{name}
        '''
        self._fit_dir = self._get_fit_dir(name=name)
    # ----------------------
    def _get_fit_dir(self, name : str) -> Path:
        '''
        Parameters
        -------------
        name: Name of directory with fits

        Returns
        -------------
        Absolute path to directory
        '''
        ana_dir = Path(os.environ['ANADIR'])

        fit_dir = ana_dir / f'fits/data/{name}'

        if not fit_dir.exists():
            raise FileNotFoundError(f'Coult not find: {fit_dir}')

        return fit_dir
    # ----------------------
    def save(self) -> None:
        '''
        Saves summary to _fit_dir/summary
        '''
        log.info(f'Saving summary to: {self._fit_dir}')

# -------------------------------
