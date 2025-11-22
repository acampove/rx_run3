import tensorflow_probability.python.distributions as tfd
from ..core.basepdf import BasePDF as BasePDF
from ..core.parameter import convert_to_parameter as convert_to_parameter
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import Space as Space, supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.deprecation import deprecated_args as deprecated_args
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from _typeshed import Incomplete
from typing import Literal
from zfit import z as z
from zfit._interfaces import ZfitData as ZfitData
from zfit.util.exception import AnalyticSamplingNotImplemented as AnalyticSamplingNotImplemented

def tfd_analytic_sample(n: int, dist: tfd.Distribution, limits: ztyping.ObsTypeInput): ...

class WrapDistribution(BasePDF):
    dist_params: Incomplete
    dist_kwargs: Incomplete
    def __init__(self, distribution, dist_params, obs, params=None, dist_kwargs=None, dtype=..., name=None, **kwargs) -> None: ...
    @property
    def distribution(self): ...

class Gauss(WrapDistribution, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Gauss', label=None) -> None: ...

class GaussPDFRepr(BasePDFRepr):
    hs3_type: Literal['Gauss']
    x: SpaceRepr
    mu: Serializer.types.ParamInputTypeDiscriminated
    sigma: Serializer.types.ParamInputTypeDiscriminated

class ExponentialTFP(WrapDistribution):
    def __init__(self, tau: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, name: str = 'Exponential', label: str | None = None) -> None: ...

class Uniform(WrapDistribution):
    def __init__(self, low: ztyping.ParamTypeInput, high: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Uniform', label: str | None = None) -> None: ...

class TruncatedGauss(WrapDistribution):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, low: ztyping.ParamTypeInput, high: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'TruncatedGauss', label: str | None = None) -> None: ...

class Cauchy(WrapDistribution, SerializableMixin):
    def __init__(self, m: ztyping.ParamTypeInput, gamma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Cauchy', label: str | None = None) -> None: ...

class CauchyPDFRepr(BasePDFRepr):
    hs3_type: Literal['Cauchy']
    x: SpaceRepr
    m: Serializer.types.ParamTypeDiscriminated
    gamma: Serializer.types.ParamTypeDiscriminated

class Poisson(WrapDistribution, SerializableMixin):
    def __init__(self, lam: ztyping.ParamTypeInput = None, obs: ztyping.ObsTypeInput = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Poisson', lamb=None, label: str | None = None) -> None: ...

class PoissonPDFRepr(BasePDFRepr):
    hs3_type: Literal['Poisson']
    x: SpaceRepr
    lam: Serializer.types.ParamTypeDiscriminated

class LogNormal(WrapDistribution, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'LogNormal', label: str | None = None) -> None: ...

class LogNormalPDFRepr(BasePDFRepr):
    hs3_type: Literal['LogNormal']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated

class ChiSquared(WrapDistribution, SerializableMixin):
    def __init__(self, ndof: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'ChiSquared', label: str | None = None) -> None: ...

class ChiSquaredPDFRepr(BasePDFRepr):
    hs3_type: Literal['ChiSquared']
    x: SpaceRepr
    ndof: Serializer.types.ParamTypeDiscriminated

class StudentT(WrapDistribution, SerializableMixin):
    def __init__(self, ndof: ztyping.ParamTypeInput, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'StudentT', label: str | None = None) -> None: ...

class StudentTPDFRepr(BasePDFRepr):
    hs3_type: Literal['StudentT']
    x: SpaceRepr
    ndof: Serializer.types.ParamTypeDiscriminated
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated

class QGauss(WrapDistribution, SerializableMixin):
    def __init__(self, q: ztyping.ParamTypeInput, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'QGauss', label: str | None = None) -> None: ...

class QGaussPDFRepr(BasePDFRepr):
    hs3_type: Literal['QGauss']
    x: SpaceRepr
    q: Serializer.types.ParamTypeDiscriminated
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated

class BifurGauss(WrapDistribution, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigmal: ztyping.ParamTypeInput, sigmar: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'BifurGauss', label: str | None = None) -> None: ...

class BifurGaussPDFRepr(BasePDFRepr):
    hs3_type: Literal['BifurGauss']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigmal: Serializer.types.ParamTypeDiscriminated
    sigmar: Serializer.types.ParamTypeDiscriminated

class Gamma(WrapDistribution, SerializableMixin):
    def __init__(self, gamma: ztyping.ParamTypeInput, beta: ztyping.ParamTypeInput, mu: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Gamma', label: str | None = None) -> None: ...

class GammaPDFRepr(BasePDFRepr):
    hs3_type: Literal['Gamma']
    x: SpaceRepr
    gamma: Serializer.types.ParamTypeDiscriminated
    beta: Serializer.types.ParamTypeDiscriminated
    mu: Serializer.types.ParamTypeDiscriminated

class JohnsonSU(WrapDistribution, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, lambd: ztyping.ParamTypeInput, gamma: ztyping.ParamTypeInput, delta: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'JohnsonSU', label: str | None = None) -> None: ...

class JohnsonSUPDFRepr(BasePDFRepr):
    hs3_type: Literal['JohnsonSU']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    lambd: Serializer.types.ParamTypeDiscriminated
    gamma: Serializer.types.ParamTypeDiscriminated
    delta: Serializer.types.ParamTypeDiscriminated

class GeneralizedGauss(WrapDistribution, SerializableMixin):
    def __init__(self, mu: ztyping.ParamTypeInput, sigma: ztyping.ParamTypeInput, beta: ztyping.ParamTypeInput, obs: ztyping.ObsTypeInput, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'GeneralizedGauss', label=None) -> None: ...

class GeneralizedGaussPDFRepr(BasePDFRepr):
    hs3_type: Literal['GeneralizedGauss']
    x: SpaceRepr
    mu: Serializer.types.ParamTypeDiscriminated
    sigma: Serializer.types.ParamTypeDiscriminated
    beta: Serializer.types.ParamTypeDiscriminated
