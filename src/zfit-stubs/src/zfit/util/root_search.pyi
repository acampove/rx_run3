from . import tff_types as types
from collections.abc import Callable

__all__ = ['BrentResults', 'brentq']

class BrentResults:
    estimated_root: types.RealTensor
    objective_at_estimated_root: types.RealTensor
    num_iterations: types.IntTensor
    converged: types.BoolTensor

class _BrentSearchConstants:
    false: types.BoolTensor
    zero: types.RealTensor
    zero_value: types.RealTensor

class _BrentSearchState:
    best_estimate: types.RealTensor
    value_at_best_estimate: types.RealTensor
    last_estimate: types.RealTensor
    value_at_last_estimate: types.RealTensor
    contrapoint: types.RealTensor
    value_at_contrapoint: types.RealTensor
    step_to_best_estimate: types.RealTensor
    step_to_last_estimate: types.RealTensor
    num_iterations: types.IntTensor
    finished: types.BoolTensor

class _BrentSearchParams:
    objective_fn: Callable[[types.BoolTensor], types.BoolTensor]
    max_iterations: types.IntTensor
    absolute_root_tolerance: types.RealTensor
    relative_root_tolerance: types.RealTensor
    function_tolerance: types.RealTensor
    stopping_policy_fn: Callable[[types.BoolTensor], types.BoolTensor]

def brentq(objective_fn: Callable[[types.RealTensor], types.RealTensor], left_bracket: types.RealTensor, right_bracket: types.RealTensor, value_at_left_bracket: types.RealTensor = None, value_at_right_bracket: types.RealTensor = None, absolute_root_tolerance: types.RealTensor = 2e-07, relative_root_tolerance: types.RealTensor = None, function_tolerance: types.RealTensor = 2e-07, max_iterations: types.IntTensor = 100, stopping_policy_fn: Callable[[types.BoolTensor], types.BoolTensor] | None = None, validate_args: bool = False, name: str | None = None) -> BrentResults: ...
