from ..core.parameter import Parameter as Parameter, assign_values as assign_values
from ..util.cache import GraphCachable as GraphCachable
from ..util.exception import MaximumIterationReached as MaximumIterationReached
from .baseminimizer import BaseMinimizer as BaseMinimizer, minimize_supports as minimize_supports
from .fitresult import Approximations as Approximations, FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from .termination import ConvergenceCriterion as ConvergenceCriterion
from _typeshed import Incomplete
from collections.abc import Mapping
from zfit._interfaces import ZfitLoss as ZfitLoss

class OptimizeStop(Exception): ...

class LevenbergMarquardt(BaseMinimizer, GraphCachable):
    mode: Incomplete
    rho_min: Incomplete
    rho_max: Incomplete
    Lup: int
    Ldn: int
    def __init__(self, tol: float | None = None, mode: int | None = None, rho_min: float | None = None, rho_max: float | None = None, verbosity: int | None = None, options: Mapping[str, object] | None = None, maxiter: int | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str | None = None) -> None: ...
