'''
Module containing FitResult class
'''
import zfit
import math

from pathlib   import Path
from pydantic  import BaseModel, ConfigDict
from dmu       import LogStore

zres = zfit.result.FitResult
zpar = zfit.param.Parameter
log  = LogStore.add_logger('rx_stats:fit_result')
# -------------------------------------
class FitParameter(BaseModel):
    '''
    Class meant to represent fitting parameter
    '''
    model_config = ConfigDict(frozen=True)

    name : str
    value: float 
    error: float
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''
        msg = f'{self.name:<30}{self.value:<30}{"±":<10}{self.error:<30}'

        return msg
# -------------------------------------
class FitResult(BaseModel):
    '''
    Class meant to represent fit result
    '''
    model_config = ConfigDict(frozen=True)

    valid      : bool
    status     : int
    parameters : tuple[FitParameter,...]
    # ----------------------
    def __getitem__(self, name : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        name: Name of parameter

        Returns
        -------------
        Tuple with fit value and error
        '''
        for par in self.parameters:
            if par.name != name:
                continue

            return par.value, par.error

        raise ValueError(f'Parameter {name} not found in:\n {self}')
    # ----------------------
    def get(self, name : str, fall_back : float) -> tuple[float,float]:
        '''
        Parameters
        ----------------
        name     : Name of parameter
        fall_back: Replacement for value and error if parameter not found

        Returns
        ----------------
        Tuple with value and error
        '''
        for par in self.parameters:
            if par.name != name:
                continue

            return par.value, par.error

        return fall_back, fall_back
    # ----------------------
    def __str__(self) -> str:
        '''
        Returns
        -------------
        String representation
        '''

        msg  = '\n'
        msg += 100 * '-' + '\n'
        msg += f'{"Name":<30}{"Value":<30}{"":<10}{"Error":<30}\n'
        msg += 100 * '-' + '\n'
        for par in self.parameters:
            msg += str(par) + '\n' 

        return msg
    # ----------------------
    def to_json(
        self, 
        path : Path) -> None:
        '''
        Parameters
        --------------
        path: Path to JSON file
        '''
        data = self.model_dump_json(indent = 2)

        log.info(f'Saving to: {path}')

        path.write_text(data = data)
    # ----------------------
    def set_pars(self, pars : set[zpar]) -> set[zpar]:
        '''
        Parameters
        -------------
        pars: Set of parameters

        Returns
        -------------
        Set of parameters with values set as in current object 
        '''
        fail = False
        for par in pars:
            value, _ = self.get(par.name, fall_back=math.nan)
            if math.isnan(value):
                fail = True
                log.error(par.name)
            else:
                par.set_value(value)

        if fail:
            raise ValueError('At least one parameter not found in result')

        return pars
    # ----------------------
    @classmethod
    def _par_from_zfit(
        cls, 
        name        : str,
        no_errors_ok: bool,
        data        : dict) -> 'FitParameter':
        '''
        Parameters
        -------------
        name        : Parameter name
        data        : Dictionary holding data from parameter fit
        no_errors_ok: If true, it will return nan when errors are missing

        Returns
        -------------
        FitParameter
        '''
        if 'minuit_hesse' in data:
            error = data['minuit_hesse']['error']
        elif no_errors_ok:
            error = math.nan
        else:
            import pprint

            pprint.pprint(data)

            raise KeyError(f'Cannot extract error from parameter: {name}')

        try:
            value = data['value']
        except KeyError:
            import pprint
            pprint.pprint(data)
            raise KeyError(f'Cannot extract value from parameter: {name}')

        return FitParameter(
            name = name,
            value= value,
            error= error)
    # ----------------------
    @classmethod
    def from_zfit(
        cls, 
        res          : zres,
        no_errors_ok : bool = False) -> 'FitResult':
        '''
        Parameters
        ---------------
        res         : Zfit fitting result
        no_errors_ok: If true, it won't raise exception if error not found

        Returns
        ---------------
        Fitting result
        '''
        l_par : list[FitParameter] = [] 
        for par, data in res.params.items():
            param = cls._par_from_zfit(
                no_errors_ok = no_errors_ok,
                name         = par.name, 
                data         = data) # type: ignore

            l_par.append(param)

        pars = tuple(l_par)

        return cls(
            status     = res.status,
            valid      = res.valid,
            parameters = pars)
    # ----------------------
    @classmethod
    def from_json(
        cls,
        path : Path) -> 'FitResult':
        '''
        Parameters
        --------------
        path: Path to JSON file
        '''
        if not path.exists():
            raise FileNotFoundError(f'File not found: {path}')

        data = path.read_text()

        return cls.model_validate_json(json_data = data)
# -------------------------------------
