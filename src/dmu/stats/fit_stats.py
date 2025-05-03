'''
Module with FitStats class
'''

import re
import pickle
from typing import Union

import numpy
import pandas                as pnd
from zfit.result            import FitResult  as zres
from dmu.logging.log_store  import LogStore

log = LogStore.add_logger('dmu:fit_stats')
# -------------------------------
class FitStats:
    '''
    Class meant to provide fit statistics
    '''
    # -------------------------------
    def __init__(self, fit_dir : str):
        '''
        fit_dir :  Path to directory where fit outputs are stored
        '''
        self._fit_dir = fit_dir
        self._df      : pnd.DataFrame

        self._regex   = r'^([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*$'
        self._sig_yld = 'nsig'
    # -------------------------------
    def _row_from_line(self, line : str) -> Union[list,None]:
        mtch = re.match(self._regex, line)
        if not mtch:
            return None

        [name, value, low, high, is_floating, mu_sg] = mtch.groups()

        if mu_sg == 'none':
            mu = numpy.nan
            sg = numpy.nan
        else:
            [mu, sg] = mu_sg.split('___')
            mu       = float(mu)
            sg       = float(sg)

        is_floating = int(is_floating)  #Direct conversion from '0' to bool will break this
        is_floating = bool(is_floating)
        row         = [name, float(value), float(low), float(high), is_floating, mu, sg]

        return row
    # -------------------------------
    def _load_data(self) -> None:
        fit_path = f'{self._fit_dir}/post_fit.txt'

        with open(fit_path, encoding='utf-8') as ifile:
            l_line = ifile.read().splitlines()

        df = pnd.DataFrame(columns=['name', 'value', 'low', 'high', 'float', 'mu', 'sg'])
        for line in l_line:
            row = self._row_from_line(line)
            if row is None:
                continue

            df.loc[len(df)] = row

        self._df = self._attach_errors(df)
    # -------------------------------
    def _error_from_res(self, name : str, res : zres) -> float:
        if name not in res.params:
            raise KeyError(f'{name} not found in {res.params}')

        d_data = res.params[name]

        return d_data['hesse']['error']
    # -------------------------------
    def _attach_errors(self, df : pnd.DataFrame) -> pnd.DataFrame:
        pkl_path = f'{self._fit_dir}/fit.pkl'
        with open(pkl_path, 'rb') as ifile:
            res = pickle.load(ifile)

        df['error'] = df['name'].apply(lambda name : self._error_from_res(name, res))

        return df
    # -------------------------------
    def get_value(self, name : str, kind : str) -> float:
        '''
        Returns float with value associated to fit
        name : Name of variable, e.g. mu, sg, nsig
        kind : Type of quantity, e.g. value, error
        '''
        self._load_data()
        log.debug('')
        log.debug(self._df)

        log.info('Retrieving signal yield')
        df   = self._df[self._df.name == name]
        nrow = len(df)
        if nrow != 1:
            raise ValueError(f'Cannot retrieve one and only one row, found {nrow}')

        val = df[kind]

        return float(val)
# -------------------------------
