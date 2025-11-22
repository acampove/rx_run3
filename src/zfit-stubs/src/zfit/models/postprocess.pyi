from .._interfaces import ZfitPDF
from ..core.serialmixin import SerializableMixin
from ..util import ztyping
from ..util.ztyping import ExtendedInputType, NormInputType
from .basefunctor import FunctorPDFRepr
from .functor import BaseFunctor
from _typeshed import Incomplete
from typing import Literal

__all__ = ['PositivePDF']

class PositivePDF(BaseFunctor, SerializableMixin):
    epsilon: Incomplete
    def __init__(self, pdf: ZfitPDF, epsilon: float = 1e-100, obs: ztyping.ObsTypeInput = None, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'PositivePDF', **kwargs) -> None: ...

class PositivePDFRepr(FunctorPDFRepr):
    hs3_type: Literal['PositivePDF']
    epsilon: float
