from ..util.exception import OperationNotAllowedError as OperationNotAllowedError
from .baseminimizer import BaseStepMinimizer as BaseStepMinimizer, minimize_supports as minimize_supports
from zfit._interfaces import ZfitIndependentParameter as ZfitIndependentParameter, ZfitLoss as ZfitLoss

class WrapOptimizer(BaseStepMinimizer):
    def __init__(self, optimizer, tol=None, criterion=None, strategy=None, verbosity=None, name=None, **kwargs) -> None: ...
