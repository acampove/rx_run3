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
# TODO: Generalize tool to deal with any sample, i.e.
# - Generalize _regex to match any sample and pick name
# - For MC, instead of nentries use yld_{sample}
# -------------------------------
class FitSummary:
    '''
    Class meant to: 

    - Collect yields an brem fractions from fits
    - Plot yields and fractions
    - Store information in JSON for further usage
    '''
    # ----------------------
    def __init__(
        self, 
        signal: str,
        name  : str) -> None:
        '''
        Parameters
        -------------
        signal: Name of signal component, e.g. jpsi
        name  : Name of directory in {ANADIR}/fits/data/{name}
        '''
        self._fit_dir = self._get_fit_dir(name=name)

        dat_regex = r'(\d{3})_(\d{3})_b(\d)/reso/(rk|rkst)/(muon|electron)/data/(psi2|jpsi)/brem_(\d{3})/parameters.json'
        sim_regex = fr'(\d{{3}})_(\d{{3}})_b(\d)/reso/(rk|rkst)/(muon|electron)/{signal}/brem_(\d{{3}})/.*_(psi2|jpsi)/category/parameters.json'

        dat_pattern = '*_*_b*/reso/r*/*/data/*/brem_*/parameters.json'
        sim_pattern = f'*_*_b*/reso/r*/*/{signal}/brem_*/*/category/parameters.json'

        self._patterns : Final[dict[str,str]]  = {'dat' : dat_pattern, 'sim' : sim_pattern}
        self._regexes  : Final[dict[str,str]]  = {'dat' : dat_regex  , 'sim' : sim_regex  }
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

        log.info(f'Picking up parameters from directory: {fit_dir}')

        return fit_dir
    # ----------------------
    def _get_parameter_paths(self, kind : str) -> list[Path]:
        '''
        Parameters
        -------------
        kind: dat or sim 

        Returns
        -------------
        List of paths to JSON files with fitting parameters
        '''
        pattern = self._patterns[kind]
        gen     = self._fit_dir.glob(pattern=pattern)
        files   = list(gen)

        if not files:
            raise ValueError(f'No files found in: {self._fit_dir}/{pattern}')
        else:
            nfile   = len(files)
            log.info(f'Found {nfile} files')

        return files
    # ----------------------
    def _attach_information(
        self, 
        kind : str,
        data : dict[str, float | str], 
        path : Path) -> dict[str, float | str]:
        '''
        Parameters
        -------------
        kind : dat or sim 
        data : Dictionary with fitting parameter information
        path : Path to JSON file with this information

        Returns
        -------------
        Input dictionary with extra information added, taken from path
        '''
        regex = self._regexes[kind]
        mtch  = re.match(f'.*/{regex}', str(path))
        if not mtch:
            raise ValueError(f'Cannot match pattern \"{regex}\" to \"{path}\"')

        groups : list[str] = list(mtch.groups())
        channel            = groups[4]
        channel            = {'electron' : 'ee', 'muon' : 'mm'}[channel]

        data['mva_cmb'] = groups[0]
        data['mva_prc'] = groups[1]
        data['block'  ] = groups[2]
        data['project'] = groups[3]
        data['channel'] = channel

        if kind == 'dat':
            data['q2bin'  ] = groups[5]
            data['brem'   ] = groups[6]
        elif kind == 'sim':
            data['q2bin'  ] = groups[6]
            data['brem'   ] = groups[5]
        else:
            raise ValueError(f'Invalid kind: {kind}')

        return data
    # ----------------------
    def _get_dataframe(
        self, 
        kind : str,
        path : Path) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        kind : dat or sim 
        path : Path to JSON file with fit parameters

        Returns
        -------------
        DataFrame
        '''
        data       = gut.load_json(path=path)
        parameters = dict()

        for name, [value, error] in data.items():
            parameters[f'{name}_value'] = value
            parameters[f'{name}_error'] = error 

        parameters = self._attach_information(data=parameters, path=path, kind=kind)
        values     = { key : [value] for key, value in parameters.items() }

        # Do this just for MC, data has parameters starting with yld_
        if all( not key.startswith('yld_') for key in parameters ):
            parameters['nentries'] = self._get_mc_nentries(path=path)

        df = pnd.DataFrame(values)

        return df
    # ----------------------
    def _get_mc_nentries(self, path : Path) -> int:
        '''
        This is needed to get entries from simulated datasets

        Parameters
        -------------
        path: Path to YAML file with parameters

        Returns
        -------------
        Number of entries in dataset used in fit
        '''
        json_path = path.parent / 'data.json'
        if not json_path.exists():
            raise FileNotFoundError(f'Cannot find: {json_path}')

        try:
            df   = pnd.read_json(json_path)
        except Exception as exc:
            raise Exception(f'Cannot build dataframe from {json_path}') from exc

        return len(df)
    # ----------------------
    def get_df(self, force_update : bool = False) -> pnd.DataFrame:
        '''
        Parameters
        ---------------
        force_update: If true (default false) will regenerate file, otherwise will reuse file if found 
        '''
        output_path  = self._fit_dir / 'parameters.parquet'
        if output_path.exists() and not force_update:
            log.info(f'Reading cached files at: {output_path}')
            return pnd.read_parquet(output_path)

        df_dat = self._get_df(kind = 'dat')
        df_sim = self._get_df(kind = 'sim')
        df     = pnd.concat([df_dat, df_sim], axis=0)
        df     = df.reset_index(drop=True)
        df     = df.astype(dtype = {'block' : int, 'brem' : int})

        self._save_df(df=df, path = output_path)

        return df
    # ----------------------
    def _get_df(self, kind : str) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        kind: dat or sim, used to pick input JSON files

        Returns
        -------------
        Pandas dataframe with parameters
        '''
        paths = self._get_parameter_paths(kind = kind)

        l_df : list[pnd.DataFrame] = []
        for path in tqdm.tqdm(paths, ascii=' -'):
            df = self._get_dataframe(path=path, kind = kind)
            l_df.append(df)

        df         = pnd.concat(objs=l_df, axis=0)
        df['kind'] = kind

        return df
    # ----------------------
    def _save_df(
        self, 
        df   : pnd.DataFrame, 
        path : Path) -> None:
        '''
        Parameters
        ----------------
        path: Path to parquet file to save
        '''
        df.to_parquet(path = path)
        log.info(f'Saving summary to: {path}')

        str_path = str(path)
        str_path = str_path.replace('.parquet', '.md')
        path     = Path(str_path)
        df.to_markdown(buf = path)

        log.info(f'Saving summary to: {path}')
# -------------------------------
