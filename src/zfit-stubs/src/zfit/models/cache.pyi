import tensorflow as tf
from .. import z as z
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import supports as supports
from ..serialization import Serializer as Serializer
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.exception import AnalyticGradientNotAvailable as AnalyticGradientNotAvailable
from .basefunctor import FunctorPDFRepr as FunctorPDFRepr
from .functor import BaseFunctor as BaseFunctor
from collections.abc import Callable as Callable
from typing import Literal

def get_value(cache: tf.Variable, flag: tf.Variable, func: Callable): ...

class CachedPDF(BaseFunctor, SerializableMixin):
    def __init__(self, pdf: ztyping.PDFInputType, *, epsilon: float | None = None, extended: ztyping.ExtendedInputType = None, norm: ztyping.NormInputType = None, cache_tol=None, name: str | None = None, label: str | None = None) -> None: ...

class CachedPDFRepr(FunctorPDFRepr):
    hs3_type: Literal['CachedPDF']
