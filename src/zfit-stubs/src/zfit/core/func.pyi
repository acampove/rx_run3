from _typeshed import Incomplete
from zfit import Data as Data
from zfit.core.values import ValueHolder as ValueHolder
from zfit.util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented, WorkInProgressError as WorkInProgressError
from zfit_interface.func import ZfitFunc

def to_value_holder(var): ...
def to_data(value, space): ...

class Func(ZfitFunc):
    var: Incomplete
    params: Incomplete
    space: Incomplete
    output_var: Incomplete
    label: Incomplete
    def __init__(self, var, output_var=None, label=None) -> None: ...
    def __call__(self, var=None): ...
    def values(self, *, var, options=None): ...
