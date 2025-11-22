from ..core.basepdf import BasePDF as BasePDF
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import ANY_LOWER as ANY_LOWER, ANY_UPPER as ANY_UPPER, Space as Space, supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..util import ztyping as ztyping
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from _typeshed import Incomplete
from typing import Literal
from zfit import z as z

def crystalball_func(x, mu, sigma, alpha, n): ...
def double_crystalball_func(x, mu, sigma, alphal, nl, alphar, nr): ...
def generalized_crystalball_func(x, mu, sigmal, alphal, nl, sigmar, alphar, nr): ...
def crystalball_integral(limits, params, model): ...
def crystalball_integral_func(mu, sigma, alpha, n, lower, upper): ...
def double_crystalball_mu_integral(limits, params, model): ...
def double_crystalball_mu_integral_func(mu, sigma, alphal, nl, alphar, nr, lower, upper): ...
def generalized_crystalball_mu_integral(limits, params, model): ...
def generalized_crystalball_mu_integral_func(mu, sigmal, alphal, nl, sigmar, alphar, nr, lower, upper): ...

class CrystalBall(BasePDF, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, alpha: ztyping.ParamTypeInput, n: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'CrystalBall', label: str | None = None) -> None: ...

class CrystalBallPDFRepr(BasePDFRepr):
    hs3_type: Literal['CrystalBall']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated
    alpha: Serializer.types.ParamTypeDiscriminated
    n: Serializer.types.ParamTypeDiscriminated

crystalball_integral_limits: Incomplete

class DoubleCB(BasePDF, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, alphal: ztyping.ParamTypeInput, nl: ztyping.ParamTypeInput, alphar: ztyping.ParamTypeInput, nr: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'DoubleCB', label: str | None = None) -> None: ...

class DoubleCBPDFRepr(BasePDFRepr):
    hs3_type: Literal['DoubleCB']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated
    alphal: Serializer.types.ParamTypeDiscriminated
    nl: Serializer.types.ParamTypeDiscriminated
    alphar: Serializer.types.ParamTypeDiscriminated
    nr: Serializer.types.ParamTypeDiscriminated

class GeneralizedCB(BasePDF, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigmal: ztyping.ParamTypeInput, alphal: ztyping.ParamTypeInput, nl: ztyping.ParamTypeInput, sigmar: ztyping.ParamTypeInput, alphar: ztyping.ParamTypeInput, nr: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'GeneralizedCB', label: str | None = None) -> None: ...

class GeneralizedCBPDFRepr(BasePDFRepr):
    hs3_type: Literal['GeneralizedCB']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigmal: Serializer.types.ParamTypeDiscriminated
    alphal: Serializer.types.ParamTypeDiscriminated
    sigmar: Serializer.types.ParamTypeDiscriminated
    nl: Serializer.types.ParamTypeDiscriminated
    alphar: Serializer.types.ParamTypeDiscriminated
    nr: Serializer.types.ParamTypeDiscriminated

def gaussexptail_func(x, mu, sigma, alpha): ...
def generalized_gaussexptail_func(x, mu, sigmal, alphal, sigmar, alphar): ...
def gaussexptail_integral(limits, params, model): ...
def gaussexptail_integral_func(mu, sigma, alpha, lower, upper): ...
def generalized_gaussexptail_integral(limits, params, model): ...
def generalized_gaussexptail_integral_func(mu, sigmal, alphal, sigmar, alphar, lower, upper): ...

class GaussExpTail(BasePDF, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, alpha: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'GaussExpTail', label: str | None = None) -> None: ...

class GaussExpTailPDFRepr(BasePDFRepr):
    hs3_type: Literal['GaussExpTail']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated
    alpha: Serializer.types.ParamTypeDiscriminated

gaussexptail_integral_limits: Incomplete

class GeneralizedGaussExpTail(BasePDF, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigmal: ztyping.ParamTypeInput, alphal: ztyping.ParamTypeInput, sigmar: ztyping.ParamTypeInput, alphar: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'GeneralizedGaussExpTail', label: str | None = None) -> None: ...

class GeneralizedGaussExpTailPDFRepr(BasePDFRepr):
    hs3_type: Literal['GeneralizedGaussExpTail']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigmal: Serializer.types.ParamTypeDiscriminated
    alphal: Serializer.types.ParamTypeDiscriminated
    sigmar: Serializer.types.ParamTypeDiscriminated
    alphar: Serializer.types.ParamTypeDiscriminated
