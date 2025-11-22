import scipy.optimize
from ..core.parameter import assign_values as assign_values
from ..util.container import convert_to_container as convert_to_container
from ..util.exception import MaximumIterationReached as MaximumIterationReached
from ..util.warnings import warn_experimental_feature as warn_experimental_feature
from .baseminimizer import BaseMinimizer as BaseMinimizer, NOT_SUPPORTED as NOT_SUPPORTED, minimize_supports as minimize_supports, print_minimization_status as print_minimization_status
from .fitresult import FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from .termination import CRITERION_NOT_AVAILABLE as CRITERION_NOT_AVAILABLE, ConvergenceCriterion as ConvergenceCriterion
from collections.abc import Callable as Callable, Mapping

class ScipyBaseMinimizer(BaseMinimizer):
    def __init__(self, method: str, tol: float | None, internal_tol: Mapping[str, float | None], gradient: Callable | str | NOT_SUPPORTED | None, hessian: None | Callable | str | scipy.optimize.HessianUpdateStrategy | NOT_SUPPORTED, maxiter: int | str | None = None, minimizer_options: Mapping[str, object] | None = None, verbosity: int | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, minimize_func: callable | None = None, initializer: Callable | None = None, verbosity_setter: Callable | None = None, name: str = 'ScipyMinimizer') -> None: ...
    @classmethod
    def __init_subclass__(cls, **kwargs) -> None: ...

class ScipyLBFGSB(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, maxcor: int | None = None, maxls: int | None = None, verbosity: int | None = None, gradient: Callable | str | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy L-BFGS-B ') -> None: ...

class ScipyBFGS(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, c1: float | None = None, c2: float | None = None, verbosity: int | None = None, gradient: Callable | str | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str | None = None) -> None: ...

class ScipyTrustKrylov(ScipyBaseMinimizer):
    @warn_experimental_feature
    def __init__(self, tol: float | None = None, inexact: bool | None = None, gradient: Callable | str | None = None, hessian: None | Callable | str | scipy.optimize.HessianUpdateStrategy = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy trust-krylov ') -> None: ...

class ScipyTrustNCG(ScipyBaseMinimizer):
    @warn_experimental_feature
    def __init__(self, tol: float | None = None, init_trust_radius: float | None = None, eta: float | None = None, max_trust_radius: int | None = None, gradient: Callable | str | None = None, hessian: None | Callable | str | scipy.optimize.HessianUpdateStrategy = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy trust-ncg ') -> None: ...

class ScipyTrustConstr(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, init_trust_radius: int | None = None, gradient: Callable | str | None = None, hessian: None | Callable | str | scipy.optimize.HessianUpdateStrategy = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy trust-constr ') -> None: ...

class ScipyNewtonCG(ScipyBaseMinimizer):
    @warn_experimental_feature
    def __init__(self, tol: float | None = None, gradient: Callable | str | None = None, hessian: None | Callable | str | scipy.optimize.HessianUpdateStrategy = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy Newton-CG ') -> None: ...

class ScipyTruncNC(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, maxcg: int | None = None, maxls: int | None = None, eta: float | None = None, rescale: float | None = None, gradient: Callable | str | None = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy Truncated Newton Conjugate ') -> None: ...

class ScipyDogleg(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, init_trust_radius: int | None = None, eta: float | None = None, max_trust_radius: int | None = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy Dogleg ') -> None: ...

class ScipyPowell(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy Powell ') -> None: ...

class ScipySLSQP(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, gradient: Callable | str | None = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy SLSQP ') -> None: ...

class ScipyCOBYLA(ScipyBaseMinimizer):
    @warn_experimental_feature
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy COBYLA ') -> None: ...

class ScipyNelderMead(ScipyBaseMinimizer):
    def __init__(self, tol: float | None = None, adaptive: bool | None = True, verbosity: int | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str = 'SciPy Nelder-Mead ') -> None: ...

def combine_optimize_results(results): ...
