from ..core.parameter import assign_values as assign_values
from ..util.exception import MaximumIterationReached as MaximumIterationReached
from .baseminimizer import BaseMinimizer as BaseMinimizer, minimize_supports as minimize_supports, print_minimization_status as print_minimization_status
from .fitresult import FitResult as FitResult
from .strategy import ZfitStrategy as ZfitStrategy
from .termination import CRITERION_NOT_AVAILABLE as CRITERION_NOT_AVAILABLE, ConvergenceCriterion as ConvergenceCriterion, EDM as EDM

class Ipyopt(BaseMinimizer):
    def __init__(self, tol: float | None = None, maxcor: int | None = None, verbosity: int | None = None, hessian: str | None = None, options: dict[str, object] | None = None, maxiter: int | str | None = None, criterion: ConvergenceCriterion | None = None, strategy: ZfitStrategy | None = None, name: str | None = 'Ipyopt') -> None: ...
