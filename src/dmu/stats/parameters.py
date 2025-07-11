'''
Module with ParameterLibrary class
'''
from importlib.resources import files

import yaml
import pandas as pnd

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:parameters')
# --------------------------------
class ParameterLibrary:
    '''
    Class meant to:

    - Connect to database (YAML file) with parameter values and make them available
    - Allow parameter values to be overriden
    '''
    df_parameters : pnd.DataFrame
    # --------------------------------
    @staticmethod
    def _load_data() -> None:
        if hasattr(ParameterLibrary, 'df_parameters'):
            return

        data_path = files('dmu_data').joinpath('stats/parameters/data.yaml')
        data_path = str(data_path)

        d_data = {'parameter' : [], 'kind' : [], 'val' : [], 'low' : [], 'high' : []}
        with open(data_path, encoding='utf-8') as ifile:
            data = yaml.safe_load(ifile)
            for kind, d_par in data.items():
                for parameter, d_kind in d_par.items():
                    val = d_kind['val' ]
                    low = d_kind['low' ]
                    high= d_kind['high']

                    d_data['parameter'].append(parameter)
                    d_data['kind'     ].append(kind     )
                    d_data['val'      ].append(val      )
                    d_data['low'      ].append(low      )
                    d_data['high'     ].append(high     )

        df = pnd.DataFrame(d_data)

        ParameterLibrary.df_parameters = df
    # --------------------------------
    @staticmethod
    def print_parameters(kind : str) -> None:
        '''
        Method taking the kind of PDF to which the parameters are associated
        and printing the values.
        '''
        df = ParameterLibrary.df_parameters
        df = df[ df['kind'] == kind ]

        print(df)
    # --------------------------------
    @staticmethod
    def get_values(kind : str, parameter : str) -> tuple[float,float,float]:
        '''
        Takes PDF and parameter names and returns default value, low value and high value
        '''
        df = ParameterLibrary.df_parameters

        df = df[df['kind']     ==     kind]
        df = df[df['parameter']==parameter]

        if len(df) != 1:
            log.info(df)
            raise ValueError(f'Could not find one and only one row for: {kind}/{parameter}')

        val = df['val'].iloc[0]
        low = df['low'].iloc[0]
        high= df['high'].iloc[0]

        return val, low, high
    # --------------------------------
    @staticmethod
    def set_values(
            parameter : str,
            kind      : str,
            val       : float,
            low       : float,
            high      : float) -> None:
        '''
        This function will override the value and range for the given parameter
        It should be typically used before using the ModelFactory class
        '''

        df = ParameterLibrary.df_parameters

        location = (df['parameter'] == parameter) & (df['kind'] == kind)

        df.loc[location, 'val' ] = val
        df.loc[location, 'low' ] = low
        df.loc[location, 'high'] = high
# --------------------------------
ParameterLibrary._load_data()
