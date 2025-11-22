from ..core.parameter import assign_values as assign_values, convert_to_parameters as convert_to_parameters, set_values as set_values
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.exception import InitNotImplemented as InitNotImplemented, MaximumIterationReached as MaximumIterationReached, MinimizeNotImplemented as MinimizeNotImplemented, MinimizeStepNotImplemented as MinimizeStepNotImplemented, MinimizerSubclassingError as MinimizerSubclassingError, ParameterNotIndependentError as ParameterNotIndependentError
from ..util.warnings import warn_changed_feature as warn_changed_feature
from .evaluation import LossEval as LossEval
from .fitresult import FitResult as FitResult
from .interface import ZfitMinimizer as ZfitMinimizer, ZfitResult as ZfitResult
from .strategy import FailMinimizeNaN as FailMinimizeNaN, PushbackStrategy as PushbackStrategy, ZfitStrategy as ZfitStrategy
from .termination import ConvergenceCriterion as ConvergenceCriterion, EDM as EDM
from _typeshed import Incomplete
from collections.abc import Callable as Callable, Mapping
from zfit._interfaces import ZfitLoss as ZfitLoss, ZfitParameter as ZfitParameter

DefaultStrategy = PushbackStrategy
status_messages: Incomplete

def minimize_supports(*, init: bool = False) -> Callable: ...

class BaseMinimizer(ZfitMinimizer):
    verbosity: Incomplete
    minimizer_options: Incomplete
    criterion: Incomplete
    name: Incomplete
    def __init__(self, tol: float | None = None, verbosity: int | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, minimizer_options: dict | None = None, maxiter: str | int | None = None, name: str | None = None) -> None: ...
    @classmethod
    def __init_subclass__(cls, **kwargs) -> None: ...
    @property
    def tol(self): ...
    @tol.setter
    def tol(self, tol) -> None: ...
    def minimize(self, loss: ZfitLoss | Callable, params: ztyping.ParamsTypeOpt | None = None, init: ZfitResult | None = None) -> FitResult: ...
    def copy(self): ...
    def get_maxiter(self, n=None): ...
    @property
    def maxiter(self): ...
    def create_evaluator(self, loss: ZfitLoss | None = None, params: ztyping.ParametersType | None = None, numpy_converter: Callable | None = None, strategy: ZfitStrategy | None = None) -> LossEval: ...
    def create_criterion(self, loss: ZfitLoss | None = None, params: ztyping.ParametersType | None = None) -> ConvergenceCriterion: ...
BaseMinimizerV1 = BaseMinimizer

class BaseStepMinimizer(BaseMinimizer):
    def step(self, loss, params: ztyping.ParamsOrNameType = None, init: FitResult = None): ...

class NOT_SUPPORTED:
    def __new__(cls, *_, **__) -> None: ...

def print_minimization_status(converged, criterion, evaluator, i, fminopt, internal_tol: Mapping[str, float] | None = None): ...
