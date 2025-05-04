'''
Module with ParameterLibrary class
'''
from importlib.resources import files

import yaml
import pandas as pnd

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

        with open(data_path, encoding='utf-8') as ifile:
            data = yaml.safe_load(ifile)

        ParameterLibrary.df_parameters = pnd.DataFrame(data)
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

ParameterLibrary._load_data()
