import abc
import tensorflow as tf
import zfit
from ..core.basepdf import BasePDF as BasePDF
from ..core.serialmixin import SerializableMixin as SerializableMixin
from ..core.space import Space as Space, supports as supports
from ..serialization import Serializer as Serializer, SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from _typeshed import Incomplete
from collections.abc import Mapping
from typing import Literal
from zfit import z as z

def rescale_minus_plus_one(x: tf.Tensor, limits: zfit.Space) -> tf.Tensor: ...

class RecursivePolynomial(BasePDF, metaclass=abc.ABCMeta):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, coeff0: tf.Tensor | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Polynomial', label: str | None = None) -> None: ...
    @property
    def apply_scaling(self): ...
    @property
    def degree(self): ...

class BaseRecursivePolynomialRepr(BasePDFRepr):
    x: SpaceRepr
    params: Mapping[str, Serializer.types.ParamTypeDiscriminated]
    apply_scaling: bool | None
    def convert_params(cls, values): ...

def create_poly(x, polys, coeffs, recurrence): ...
def do_recurrence(x, polys, degree, recurrence): ...

legendre_polys: Incomplete

def legendre_recurrence(p1, p2, n, x): ...
def legendre_shape(x, coeffs): ...
def legendre_integral(limits: ztyping.SpaceType, norm: ztyping.SpaceType, params: list[zfit.Parameter], model: RecursivePolynomial): ...

class Legendre(RecursivePolynomial, SerializableMixin):
    def __init__(self, obs: ztyping.ObsTypeInput, coeffs: list[ztyping.ParamTypeInput], apply_scaling: bool = True, coeff0: ztyping.ParamTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Legendre', label: str | None = None) -> None: ...

class LegendreRepr(BaseRecursivePolynomialRepr):
    hs3_type: Literal['Legendre']

legendre_limits: Incomplete
chebyshev_polys: Incomplete

def chebyshev_recurrence(p1, p2, _, x): ...
def chebyshev_shape(x, coeffs): ...

class Chebyshev(RecursivePolynomial, SerializableMixin):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, coeff0: ztyping.ParamTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Chebyshev', label: str | None = None) -> None: ...

class ChebyshevRepr(BaseRecursivePolynomialRepr):
    hs3_type: Literal['Chebyshev']

def func_integral_chebyshev1(limits, norm, params, model): ...

chebyshev1_limits_integral: Incomplete
chebyshev2_polys: Incomplete

def chebyshev2_shape(x, coeffs): ...

class Chebyshev2(RecursivePolynomial, SerializableMixin):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, coeff0: ztyping.ParamTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Chebyshev2', label: str | None = None) -> None: ...

class Chebyshev2Repr(BaseRecursivePolynomialRepr):
    hs3_type: Literal['Chebyshev2']

def func_integral_chebyshev2(limits, norm, params, model): ...

chebyshev2_limits_integral: Incomplete

def generalized_laguerre_polys_factory(alpha: float = 0.0): ...

laguerre_polys: Incomplete

def generalized_laguerre_recurrence_factory(alpha: float = 0.0): ...

laguerre_recurrence: Incomplete

def generalized_laguerre_shape_factory(alpha: float = 0.0): ...

laguerre_shape: Incomplete
laguerre_shape_alpha_minusone: Incomplete

class Laguerre(RecursivePolynomial, SerializableMixin):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, coeff0: ztyping.ParamTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Laguerre', label: str | None = None) -> None: ...

class LaguerreRepr(BaseRecursivePolynomialRepr):
    hs3_type: Literal['Laguerre']

def func_integral_laguerre(limits, norm, params: dict, model): ...

laguerre_limits_integral: Incomplete
hermite_polys: Incomplete

def hermite_recurrence(p1, p2, n, x): ...
def hermite_shape(x, coeffs): ...

class Hermite(RecursivePolynomial, SerializableMixin):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, coeff0: ztyping.ParamTypeInput | None = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Hermite', label: str | None = None) -> None: ...

class HermiteRepr(BaseRecursivePolynomialRepr):
    hs3_type: Literal['Hermite']

def func_integral_hermite(limits, norm, params, model): ...

hermite_limits_integral: Incomplete

def rescale_zero_one(x, limits): ...
def de_casteljau(x, coeffs): ...
def bernstein_shape(x, coeffs): ...

class Bernstein(BasePDF, SerializableMixin):
    def __init__(self, obs, coeffs: list, apply_scaling: bool = True, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str = 'Bernstein', label: str | None = None) -> None: ...
    @property
    def apply_scaling(self): ...
    @property
    def degree(self): ...

class BernsteinPDFRepr(BasePDFRepr):
    hs3_type: Literal['Bernstein']
    x: SpaceRepr
    params: Mapping[str, Serializer.types.ParamTypeDiscriminated]
    apply_scaling: bool | None
    def convert_params(cls, values): ...

def bernstein_integral_from_xmin_to_x(x, coeffs, limits): ...
def func_integral_bernstein(limits, params, model): ...

bernstein_limits_integral: Incomplete

def convert_coeffs_dict_to_list(coeffs: Mapping) -> list: ...
