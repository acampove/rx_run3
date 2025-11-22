from ..core.basepdf import BasePDF as BasePDF
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import ANY_LOWER as ANY_LOWER, ANY_UPPER as ANY_UPPER, Space as Space, supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..util import ztyping as ztyping
from ..util.exception import BreakingAPIChangeError as BreakingAPIChangeError
from ..util.warnings import warn_advanced_feature as warn_advanced_feature
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from _typeshed import Incomplete
from typing import Literal
from zfit import z as z

class Exponential(BasePDF, SerializableMixin):
    def __init__(self, lam=None, obs: ztyping.ObsTypeInput = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Exponential', lambda_=None, label: str | None = None) -> None: ...

def exp_icdf(x, params, model): ...

limits: Incomplete

class ExponentialPDFRepr(BasePDFRepr):
    hs3_type: Literal['Exponential']
    x: SpaceRepr
    lam: Serializer.types.ParamTypeDiscriminated

class Voigt(BasePDF, SerializableMixin):
    def __init__(self, m: ztyping.ObsTypeInput, sigma: ztyping.ParamTypeInput, gamma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Voigt', label: str | None = None) -> None: ...

class VoigtPDFRepr(BasePDFRepr):
    hs3_type: Literal['Voigt']
    x: SpaceRepr
    m: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated
    gamma: Serializer.types.ParamTypeDiscriminated
