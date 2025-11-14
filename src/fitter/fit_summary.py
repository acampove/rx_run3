'''
Module holding FitSummary class
'''
import os
import re
import tqdm
import pandas as pnd

from pathlib import Path
from typing  import Final

from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut

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

        # This captures seven fields
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
    # ----------------------
    def _attach_information(self, data : dict[str, float | str], path : Path) -> dict[str, float | str]:
        '''
        Parameters
        -------------
        data: Dictionary with fitting parameter information
        path: Path to JSON file with this information

        Returns
        -------------
        Input dictionary with extra information added, taken from path
        '''
        mtch = re.match(f'.*/{self._regex}', str(path))
        if not mtch:
            raise ValueError(f'Cannot match pattern \"{self._regex}\" to \"{path}\"')

        groups : list[str] = list(mtch.groups())

        data['mva_cmb'] = groups[0]
        data['mva_prc'] = groups[1]
        data['block'  ] = groups[2]
        data['project'] = groups[3]
        data['channel'] = groups[4]
        data['q2bin'  ] = groups[5]
        data['brem'   ] = groups[6]

        return data
    # ----------------------
    def _get_dataframe(self, path : Path) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        path: Path to JSON file with fit parameters

        Returns
        -------------
        DataFrame
        '''
        data       = gut.load_json(path=path)
        parameters = dict()

        for name, [value, error] in data.items():
            parameters[f'{name}_value'] = value
            parameters[f'{name}_error'] = error 

        parameters = self._attach_information(data=parameters, path=path)
        values     = { key : [value] for key, value in parameters.items() }
        df         = pnd.DataFrame(values)

        return df
    # ----------------------
    def save(self) -> None:
        '''
        Saves summary to _fit_dir/summary
        '''
        paths = self._get_parameter_paths()

        l_df : list[pnd.DataFrame] = []
        for path in tqdm.tqdm(paths, ascii=' -'):
            df = self._get_dataframe(path=path)
            l_df.append(df)

        df = pnd.concat(objs=l_df, axis=0)
        df = df.reset_index(drop=True)

        df.to_parquet(path = self._fit_dir / 'parameters.parquet')
        df.to_markdown(buf = self._fit_dir / 'parameters.md')

        log.info(f'Saving summary to: {self._fit_dir}')
# -------------------------------
