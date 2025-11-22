import tensorflow as tf
from .. import exception as exception, z as z
from ..core.data import Data as Data, sum_samples as sum_samples
from ..core.sample import accept_reject_sample as accept_reject_sample
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..util import ztyping as ztyping
from ..util.exception import ShapeIncompatibleError as ShapeIncompatibleError, WorkInProgressError as WorkInProgressError
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from ..z.interpolate_spline import interpolate_spline as interpolate_spline
from .functor import BaseFunctor as BaseFunctor
from _typeshed import Incomplete
from typing import Literal
from zfit._interfaces import ZfitPDF as ZfitPDF

LimitsTypeInput: Incomplete

class FFTConvPDFV1(BaseFunctor, SerializableMixin):
    def __init__(self, func: ZfitPDF, kernel: ZfitPDF, n: int | None = None, limits_func: LimitsTypeInput | None = None, limits_kernel: ztyping.LimitsType | None = None, interpolation: str | None = None, obs: ztyping.ObsTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'FFTConvV1', label: str | None = None) -> None: ...
    @property
    def conv_interpolation(self): ...

class FFTConvPDFV1Repr(BasePDFRepr):
    hs3_type: Literal['FFTConvPDFV1']
    func: Serializer.types.PDFTypeDiscriminated
    kernel: Serializer.types.PDFTypeDiscriminated
    n: int | None
    limits_func: SpaceRepr | None
    limits_kernel: SpaceRepr | None
    interpolation: str | None
    obs: SpaceRepr | None
    def validate_all(cls, values): ...

class AddingSampleAndWeights:
    func: Incomplete
    kernel: Incomplete
    limits_func: Incomplete
    limits_kernel: Incomplete
    def __init__(self, func, kernel, limits_func, limits_kernel) -> None: ...
    def __call__(self, n_to_produce: int | tf.Tensor, limits, dtype): ...
