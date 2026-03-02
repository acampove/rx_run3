'''
Module with ParameterLibrary class
'''
import math
import yaml

from pathlib               import Path
from contextvars           import ContextVar
from contextlib            import contextmanager
from importlib.resources   import files
from zfit.param            import Parameter as zpar

from dmu                   import LogStore
from .configs              import YieldsConf
from .configs              import SimpleYieldConf
from .configs              import CompositeYieldConf
from .imports              import zfit

log      = LogStore.add_logger('dmu:parameters')
_YLD_CFG : ContextVar[YieldsConf | None] = ContextVar('_YLD_CFG', default = None)
# --------------------------------
class ParameterLibrary:
    '''
    Class meant to:

    - Connect to database (YAML file) with parameter values and make them available
    - Allow parameter values to be overriden
    '''
    _d_par  : dict[str,zpar      ] = {}   # When building parameters, they will be stored here, such that they can be reused, for simultaneous fits
    _values : dict[str,YieldsConf] = {}   # Values of parameters for all PDFs are in YAML file, they will be loaded here
    # --------------------------------
    @classmethod
    def _load_data(cls) -> None:
        if hasattr(cls, '_values'):
            return

        data_path = files('dmu_data').joinpath('stats/parameters/data.yaml')
        data_path = Path(str(data_path))
        data      = yaml.safe_load(data_path.read_text())

        cls._values = { pdf_name : YieldsConf(**pdf_data) 
            for pdf_name, pdf_data in data.items()} 
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
            raise ValueError(f'Cannot find PDF of kind: {kind}')

        if parameter not in cls._values[kind]:
            raise ValueError(f'For PDF {kind}, cannot find parameter: {parameter}')

        par = cls._values[kind][parameter].root
        if not isinstance(par, SimpleYieldConf):
            raise ValueError(f'Parameter {parameter} is not elementary: {par}')

        return par.val, par.min, par.max 
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
        old_par = cls._values[kind][parameter].root

        cls._values[kind][parameter].root = SimpleYieldConf(
            val = val, 
            min = low, 
            max = high)

        @contextmanager
        def _context():
            try:
                yield
            finally:
                cls._values[kind][parameter].root = old_par 

        return _context()
    # ----------------------
    @classmethod
    def parameter_schema(cls, cfg : YieldsConf):
        '''
        This context manager sets `_YLD_CFG`, which defines

        - How parameters are related. I.e. if they are multiplied
        - What their values are

        Parameters
        -------------
        cfg: DictConfig representing the values and relationships between paramaters
        '''
        token = _YLD_CFG.set(cfg)

        @contextmanager
        def _context():
            try:
                yield
            finally:
                _YLD_CFG.reset(token)

        return _context()
    # ----------------------
    @classmethod
    def get_yield(cls, name : str) -> zpar:
        '''
        Parameters
        -------------
        name: Name of parameter

        Returns
        -------------
        Zfit parameter
        '''
        log.debug(f'Picking up parameter: {name}')
        if name in cls._d_par:
            return cls._d_par[name]

        if cls._yld_cfg is None:
            raise ValueError('Parameter schema not set')

        yld_cfg = cls._yld_cfg
        if name not in yld_cfg:
            log.error(OmegaConf.to_yaml(yld_cfg))
            raise ValueError(f'Parameter {name} not found in configuration')

        this_yld = yld_cfg[name]

        if 'mul' in this_yld:
            l_par    = [ cls.get_yield(name=comp_name) for comp_name in this_yld.mul ]
            comp_par = cls._multiply_pars(name=name, pars=l_par)
            cls._d_par[name] = comp_par

            return comp_par

        if 'dif' in this_yld:
            par_1 = cls.get_yield(name=this_yld.dif[0])
            par_2 = cls.get_yield(name=this_yld.dif[1])
            comp_par = cls._subtract_pars(name = name, par_1=par_1, par_2=par_2)
            cls._d_par[name] = comp_par

            return comp_par

        # This is a non-alias, non-scl parameter, e.g. simple one
        if 'scl' not in yld_cfg[name]:
            par = cls._parameter_from_conf(name=name, cfg=yld_cfg)
            cls._d_par[name] = par
            return par

        # scl and non alias
        l_par    = [ cls.get_yield(name=comp_name) for comp_name in yld_cfg[name]['scl'] ]
        par      = cls._parameter_from_conf(name=name, cfg=yld_cfg, is_scale=True)
        l_par.append(par)
        comp_par = cls._multiply_pars(name=name, pars=l_par)

        return comp_par
    # ----------------------
    @classmethod
    def _subtract_pars(cls, name : str, par_1 : zpar, par_2 : zpar) -> zpar:
        '''
        Parameters
        -------------
        name   : Name of product parameter
        par_1/2: Parameters to be subtracted 

        Returns
        -------------
        Product of parameters
        '''
        comp_par = zfit.ComposedParameter(name, lambda pars : pars[0] - pars[1], params=[par_1, par_2])

        return comp_par
    # ----------------------
    @classmethod
    def _multiply_pars(cls, name : str, pars : list[zpar]) -> zpar:
        '''
        Parameters
        -------------
        name: Name of product parameter
        pars: List of parameters

        Returns
        -------------
        Product of parameters
        '''
        if len(pars) == 0:
            raise ValueError(f'No factor parameters found for: {name}')

        comp_par = zfit.ComposedParameter(name, lambda pars : math.prod(pars), params=pars)

        return comp_par
    # ----------------------
    @classmethod
    def _parameter_from_conf(cls, name : str, cfg : DictConfig, is_scale : bool = False) -> zpar:
        '''
        Parameters
        -------------
        name    : Name of parameter to be returned
        cfg     : Config defining values of parameter bounds and default value of parameter
        is_scale: If True, this parameter is a scale, not an actual yield. Scales on yield nPar is called s_nPar

        Returns
        -------------
        A zfit parameter
        '''
        val = cfg[name]['val']
        minv= cfg[name]['min']
        maxv= cfg[name]['max']

        preffix  = cfg[name].get('preffix', 's')
        par_name = f'{preffix}_{name}' if is_scale else name

        if minv > maxv:
            raise ValueError(f'For parameter {name}, minimum edge is larger: {minv:.3e} > {maxv:.3e}')

        if math.isclose(minv, maxv, rel_tol=1e-5):
            par = zfit.Parameter(par_name, val, minv, maxv + 1, floating=False)
        else:
            par = zfit.Parameter(par_name, val, minv, maxv + 0, floating= True)

        return par
# --------------------------------
ParameterLibrary._load_data()
