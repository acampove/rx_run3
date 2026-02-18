'''
Module containing FitResult class
'''
import math
import numpy

from typing      import Final
from scipy.stats import chi2 as chi2_dist
from pathlib     import Path
from pydantic    import BaseModel, ConfigDict, model_validator
from dmu         import LogStore
from .imports    import zfit

zres = zfit.result.FitResult
zpar = zfit.param.Parameter
log  = LogStore.add_logger('rx_stats:fit_result')

# Relative tolerance used to validate chi2
RTOL     : Final[float] = 1e-7
MIN_NDOF : Final[int  ] =  9
MAX_NDOF : Final[int  ] = 41
# -------------------------------------
class GoodnessOfFit(BaseModel):
    model_config = ConfigDict(frozen=True)

    chi2: float = math.nan 
    pval: float = math.nan 
    ndof: int
    # --------------------------
    @model_validator(mode='before')
    @classmethod
    def compute_or_validate(cls, data):
        chi2 = data.get('chi2', math.nan) 
        pval = data.get('pval', math.nan) 
        ndof = data['ndof']

        if not (MIN_NDOF < ndof < MAX_NDOF):
            raise ValueError(f'Invalid Ndof: {ndof}')

        if math.isnan(chi2) and math.isnan(pval):
            raise ValueError('Either pvalue or chi2 are needed')

        computed_pval = float(1 - chi2_dist.cdf(chi2, ndof))
        computed_chi2 = float(chi2_dist.isf(pval, ndof))
        # ----------------
        # Got pvalue, not chi2, assign chi2
        # ----------------
        if   math.isnan(chi2) and not math.isnan(computed_chi2):
            data['chi2'] = computed_chi2
            return data 
        elif math.isnan(chi2) and     math.isnan(computed_chi2):
            raise ValueError('Cannot compute chi2')
        # ----------------
        # Got chi2, not pvalue, assign pvalue 
        # ----------------
        if   math.isnan(pval) and not math.isnan(computed_pval):
            data['pval'] = computed_pval
            return data 
        elif math.isnan(pval) and     math.isnan(computed_pval):
            raise ValueError('Cannot compute pvalue')
        # ----------------
        # Got chi2 and pvalue, validate
        # ----------------
        if not math.isnan(chi2) and not numpy.isclose(chi2, computed_chi2, rtol = RTOL):
            raise ValueError(f'Inconsistent input values: {data}')

        if not math.isnan(pval) and not numpy.isclose(pval, computed_pval, rtol = RTOL):
            raise ValueError(f'Inconsistent input values: {data}')
        # ----------------

        return data 
    # --------------------------
    def __str__(self) -> str:
        fchi2 = -1 if self.chi2 is None else self.chi2
        fpval = -1 if self.pval is None else self.pval
        indof = self.ndof

        msg = f'chi2: {fchi2:.0f}\n'
        msg+= f'pval: {fpval:.3f}\n'
        msg+= f'Ndof: {indof:}\n'

        return msg 
    # --------------------------
    def __lt__(self, other : 'GoodnessOfFit') -> bool:
        '''
        Higher chi2 means worse goodness of fit
        '''
        return self.chi2 > other.chi2
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
    gof        : GoodnessOfFit | None = None
    # ----------------------
    def __lt__(self, other : 'FitResult') -> bool:
        '''
        Better fit is greater
        '''
        if self.gof is None:
            raise ValueError('No GOF defined for this instance')

        if other.gof is None:
            raise ValueError('No GOF defined for other instance')

        return self.gof < other.gof
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
