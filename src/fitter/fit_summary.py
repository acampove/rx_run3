'''
Module holding FitSummary class
'''
import os
import tqdm
import pandas as pnd

from pathlib import Path
from typing  import Final

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

        self._regex   : Final[str] = r'(\d{3})_(\d{3})_b(\d)/reso/(rk|rkst)/(muon|electron)/data/(psi2|jpsi)/brem_(\d{3})/parameters.json'
        self._pattern : Final[str] = '*_*_b*/reso/r*/*/data/*/brem_*/parameters.json'
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
    def _get_parameter_paths(self) -> list[Path]:
        '''
        Returns
        -------------
        List of paths to JSON files with fitting parameters
        '''
        gen     = self._fit_dir.glob(pattern=self._pattern)
        files   = list(gen)
        if not files:
            raise ValueError(f'No files found in: {self._fit_dir}')
        else:
            nfile   = len(files)
            log.info(f'Found {nfile} files')

        return files
    def save(self) -> None:
        '''
        Saves summary to _fit_dir/summary
        '''
        paths = self._get_parameter_paths()

        l_df : list[pnd.DataFrame] = []
        for path in tqdm.tqdm(paths, ascii=' -'):
            df = _get_dataframe(path=path)
            l_df.append(df)

        df = pnd.concat(objs=l_df, axis=1)

        log.info(f'Saving summary to: {self._fit_dir}')
# -------------------------------
