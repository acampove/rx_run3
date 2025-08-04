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
    @classmethod
    def get_values(cls, kind : str, parameter : str) -> tuple[float,float,float]:
        '''
        Parameters
        --------------
        kind     : Kind of PDF, e.g. gaus, cbl, cbr, suj
        parameter: Name of parameter for PDF, e.g. mu, sg

        Returns
        --------------
        Tuple with central value, minimum and maximum
        '''
        if kind not in cls._values:
            raise ValueError('Cannot find PDF of kind: {kind}')

        if parameter not in cls._values[kind]:
            raise ValueError(f'For PDF {kind}, cannot find parameter: {parameter}')

        val = cls._values[kind][parameter]['val' ]
        low = cls._values[kind][parameter]['low' ]
        hig = cls._values[kind][parameter]['high']

        return val, low, hig
    # --------------------------------
    @classmethod
    def values(
        cls,
        kind      : str,
        parameter : str,
        val       : float,
        low       : float,
        high      : float):
        '''
        This function will override the value and range for the given parameter
        It should be typically used before using the ModelFactory class
        '''
        old_val, old_low, old_high   = cls.get_values(kind=kind, parameter=parameter)
        cls._values[kind][parameter] = {'val' : val, 'low' : low, 'high' : high}

        @contextmanager
        def _context():
            try:
                yield
            finally:
                cls._values[kind][parameter] = {'val' : old_val, 'low' : old_low, 'high' : old_high}

        return _context()
# --------------------------------
ParameterLibrary._load_data()
