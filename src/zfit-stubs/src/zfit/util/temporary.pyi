import types
from _typeshed import Incomplete
from collections.abc import Callable as Callable
from typing import Any

class TemporarilySet:
    setter: Incomplete
    getter: Incomplete
    value: Incomplete
    old_value: Incomplete
    def __init__(self, value: Any, setter: Callable, getter: Callable) -> None: ...
    def __enter__(self): ...
    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: types.TracebackType | None) -> None: ...
