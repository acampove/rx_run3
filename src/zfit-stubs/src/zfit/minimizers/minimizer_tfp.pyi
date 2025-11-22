from ..core.parameter import assign_values as assign_values
from .baseminimizer import BaseMinimizer as BaseMinimizer, minimize_supports as minimize_supports
from .evaluation import print_gradient as print_gradient
from .fitresult import FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from _typeshed import Incomplete
from collections.abc import Mapping

class BFGS(BaseMinimizer):
    options: Incomplete
    max_calls: Incomplete
    def __init__(self, strategy: ZfitStrategy = None, tol: float = 1e-05, verbosity: int = 5, max_calls: int = 3000, name: str = 'BFGS_TFP', options: Mapping | None = None) -> None: ...
