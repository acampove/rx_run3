from .. import z as z
from ..core.parameter import Parameter as Parameter, assign_values as assign_values
from ..util.cache import GraphCachable as GraphCachable
from ..util.deprecation import deprecated_args as deprecated_args
from ..util.exception import MaximumIterationReached as MaximumIterationReached
from .baseminimizer import BaseMinimizer as BaseMinimizer, minimize_supports as minimize_supports, print_minimization_status as print_minimization_status
from .fitresult import FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from .termination import ConvergenceCriterion as ConvergenceCriterion, EDM as EDM
from _typeshed import Incomplete
from collections.abc import Mapping
from zfit._interfaces import ZfitLoss as ZfitLoss

class Minuit(BaseMinimizer, GraphCachable):
    minuit_grad: Incomplete
    def __init__(self, tol: float | None = None, mode: int | None = None, gradient: bool | str | None = None, verbosity: int | None = None, options: Mapping[str, object] | None = None, maxiter: int | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str | None = None, use_minuit_grad: bool | None = None, minuit_grad=None, minimize_strategy=None, ncall=None, minimizer_options=None) -> None: ...
    def copy(self): ...
