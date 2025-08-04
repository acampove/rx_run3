'''
Module with ParameterLibrary class
'''
from importlib.resources import files

import yaml
import pandas as pnd

from dmu.generic.typing_utilities import numeric_from_series
from dmu.logging.log_store        import LogStore

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
        Parameters
        --------------
        kind     : Kind of PDF, e.g. gaus, cbl, cbr, suj
        parameter: Name of parameter for PDF, e.g. mu, sg

        Returns
        --------------
        Tuple with central value, minimum and maximum
        '''
        df_a = ParameterLibrary.df_parameters # type: pnd.DataFrame
        df_k = df_a[df_a['kind']     ==     kind]
        df_p = df_k[df_k['parameter']==parameter]

        if len(df_p) != 1:
            log.info(df_p)
            raise ValueError(f'Could not find one and only one row for: {kind}/{parameter}')

        row = df_p.iloc[0]

        val = numeric_from_series(row=row, name='val' , numeric=float)
        low = numeric_from_series(row=row, name='low' , numeric=float)
        hig = numeric_from_series(row=row, name='high', numeric=float)

        return val, low, hig
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
