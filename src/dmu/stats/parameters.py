'''
Module with ParameterLibrary class
'''
from contextlib          import contextmanager
from importlib.resources import files

from omegaconf                    import DictConfig, OmegaConf
from dmu.logging.log_store        import LogStore

log=LogStore.add_logger('dmu:parameters')
# --------------------------------
class ParameterLibrary:
    '''
    Class meant to:

    - Connect to database (YAML file) with parameter values and make them available
    - Allow parameter values to be overriden
    '''
    _values : DictConfig
    # --------------------------------
    @classmethod
    def _load_data(cls) -> None:
        if hasattr(cls, '_values'):
            return

        data_path = files('dmu_data').joinpath('stats/parameters/data.yaml')
        data_path = str(data_path)

        values = OmegaConf.load(data_path)
        if not isinstance(values, DictConfig):
            raise TypeError(f'Wrong (not dictionary) data loaded from: {data_path}')

        cls._values = values
    # --------------------------------
    @classmethod
    def print_parameters(cls, kind : str) -> None:
        '''
        Method taking the kind of PDF to which the parameters are associated
        and printing the values.
        '''
        cfg = cls._values
        if kind not in cfg:
            raise ValueError(f'Cannot find parameters for PDF of kind: {kind}')

        log.info(cfg[kind])
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
