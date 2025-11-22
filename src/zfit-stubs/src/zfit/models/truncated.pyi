from .. import z as z
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import Space as Space, convert_to_space as convert_to_space, supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.exception import AnalyticIntegralNotImplemented as AnalyticIntegralNotImplemented, SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from .basefunctor import FunctorPDFRepr as FunctorPDFRepr
from .functor import BaseFunctor as BaseFunctor
from collections.abc import Iterable
from typing import Literal
from zfit._interfaces import ZfitPDF as ZfitPDF, ZfitSpace as ZfitSpace

def check_limits(limits: ZfitSpace | list[ZfitSpace], obs=None) -> tuple[ZfitSpace, ...]: ...
def check_overlap(limits: Iterable[ZfitSpace]) -> tuple[ZfitSpace, ...]: ...

class TruncatedPDF(BaseFunctor, SerializableMixin):
    def __init__(self, pdf: ZfitPDF, limits: ZfitSpace | Iterable[ZfitSpace], obs: ztyping.ObsTypeInput = None, *, extended: ztyping.ExtendedInputType = None, norm: ztyping.NormRangeTypeInput = None, name: str | None = None, label: str | None = None) -> None: ...
    @property
    def limits(self): ...

class TruncatedPDFRepr(FunctorPDFRepr):
    hs3_type: Literal['TruncatedPDF']
    limits: list[SpaceRepr]
