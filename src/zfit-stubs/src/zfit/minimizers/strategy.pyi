import abc
import numpy as np
from ..util import ztyping as ztyping
from .fitresult import FitResult as FitResult
from _typeshed import Incomplete
from abc import abstractmethod
from collections.abc import Mapping
from zfit._interfaces import ZfitLoss as ZfitLoss, ZfitParameter as ZfitParameter

class FailMinimizeNaN(Exception): ...

class ZfitStrategy(abc.ABC, metaclass=abc.ABCMeta):
    @abstractmethod
    def minimize_nan(self, loss: ZfitLoss, params: ztyping.ParamTypeInput, values: Mapping | None = None) -> float: ...
    @abstractmethod
    def callback(self, value: float | None, gradient: np.ndarray | None, hessian: np.ndarray | None, params: list[ZfitParameter], loss: ZfitLoss) -> tuple[float, np.ndarray, np.ndarray]: ...

class BaseStrategy(ZfitStrategy):
    fit_result: Incomplete
    error: Incomplete
    def __init__(self) -> None: ...
    def minimize_nan(self, loss: ZfitLoss, params: ztyping.ParamTypeInput, values: Mapping | None = None) -> float: ...
    def callback(self, value, gradient, hessian, params, loss): ...

class ToyStrategyFail(BaseStrategy):
    fit_result: Incomplete
    def __init__(self) -> None: ...
    def minimize_nan(self, loss: ZfitLoss, params: ztyping.ParamTypeInput, values: Mapping | None = None) -> float: ...

def make_pushback_strategy(nan_penalty: float | int = 100, nan_tol: int = 30, base: object | ZfitStrategy = ...): ...

PushbackStrategy: Incomplete

class DefaultToyStrategy(PushbackStrategy, ToyStrategyFail): ...
