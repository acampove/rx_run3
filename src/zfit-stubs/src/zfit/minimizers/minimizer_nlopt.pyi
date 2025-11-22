from ..core.parameter import assign_values as assign_values
from ..util.checks import RuntimeDependency as RuntimeDependency
from ..util.exception import MaximumIterationReached as MaximumIterationReached
from .baseminimizer import BaseMinimizer as BaseMinimizer, NOT_SUPPORTED as NOT_SUPPORTED, minimize_supports as minimize_supports, print_minimization_status as print_minimization_status
from .fitresult import FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from .termination import CRITERION_NOT_AVAILABLE as CRITERION_NOT_AVAILABLE, ConvergenceCriterion as ConvergenceCriterion, EDM as EDM
from collections.abc import Callable as Callable, Mapping

class NLoptBaseMinimizer(BaseMinimizer):
    def __init__(self, algorithm: int, tol: float | None = None, gradient: Callable | str | NOT_SUPPORTED | None = ..., hessian: Callable | str | NOT_SUPPORTED | None = ..., maxiter: int | str | None = None, minimizer_options: Mapping[str, object] | None = None, internal_tols: Mapping[str, float | None] | None = None, verbosity: int | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt Base Minimizer ') -> None: ...

class NLoptLBFGS(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, maxcor: int | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt L-BFGS ') -> None: ...

class NLoptShiftVar(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, maxcor: int | None = None, rank: int | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt Shifted Variable Memory') -> None: ...

class NLoptTruncNewton(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, maxcor: int | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt Truncated Newton') -> None: ...

class NLoptSLSQP(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt SLSQP') -> None: ...

class NLoptBOBYQA(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt BOBYQA') -> None: ...

class NLoptMMA(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt MMA') -> None: ...

class NLoptCCSAQ(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt CCSAQ') -> None: ...

class NLoptCOBYLA(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt COBYLA') -> None: ...

class NLoptSubplex(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt Subplex') -> None: ...

class NLoptMLSL(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, population: int | None = None, randomized: bool | None = None, local_minimizer: int | Mapping[str, object] | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt MLSL') -> None: ...

class NLoptStoGO(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, randomized: bool | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt MLSL') -> None: ...

class NLoptESCH(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt ESCH') -> None: ...

class NLoptISRES(NLoptBaseMinimizer):
    def __init__(self, tol: float | None = None, population: int | None = None, verbosity: int | None = None, maxiter: int | str | None = None, strategy: ZfitStrategy | None = None, criterion: ConvergenceCriterion | None = None, name: str = 'NLopt ISRES') -> None: ...
