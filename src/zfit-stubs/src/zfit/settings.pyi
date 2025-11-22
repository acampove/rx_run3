from .util.execution import RunManager as RunManager
from _typeshed import Incomplete

run: Incomplete

def set_seed(seed: int | None = None, numpy: int | bool | None = None, backend: int | bool | None = None, zfit: int | bool | None = None) -> dict[str, int | None]: ...
def generate_urandom_seed() -> int: ...
def set_verbosity(verbosity: int) -> None: ...
def get_verbosity() -> int: ...

ztypes: Incomplete
upcast_ztypes: Incomplete
options: Incomplete
advanced_warnings: Incomplete
changed_warnings: Incomplete
