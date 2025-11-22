from ..core.space import Space as Space
from ..util.exception import WorkInProgressError as WorkInProgressError
from .serializer import BaseRepr as BaseRepr
from typing import Literal

NumericTyped = float | int
NameObsTyped = tuple[str] | str | None

class SpaceRepr(BaseRepr):
    hs3_type: Literal['Space']
    name: str
    lower: NumericTyped | None
    upper: NumericTyped | None
    binning: float | None
    def validate_binning(cls, v): ...
