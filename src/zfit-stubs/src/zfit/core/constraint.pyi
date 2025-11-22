import abc
from .. import z as z
from ..serialization.serializer import BaseRepr as BaseRepr, Serializer as Serializer
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.deprecation import deprecated_args as deprecated_args
from ..util.exception import ShapeIncompatibleError as ShapeIncompatibleError
from .baseobject import BaseNumeric as BaseNumeric
from .serialmixin import SerializableMixin as SerializableMixin
from _typeshed import Incomplete
from collections.abc import Callable as Callable, Iterable, Mapping
from typing import Literal
from zfit._interfaces import ZfitConstraint as ZfitConstraint, ZfitParameter as ZfitParameter

tfd: Incomplete

class BaseConstraintRepr(BaseRepr):
    hs3_type: Literal['BaseConstraint']

class BaseConstraint(ZfitConstraint, BaseNumeric, metaclass=abc.ABCMeta):
    def __init__(self, params: dict[str, ZfitParameter] | None = None, name: str = 'BaseConstraint', dtype=..., **kwargs) -> None: ...
    def value(self): ...

class SimpleConstraint(BaseConstraint):
    def __init__(self, func: Callable, params: Mapping[str, ztyping.ParameterType] | Iterable[ztyping.ParameterType] | ztyping.ParameterType | None, *, name: str | None = None) -> None: ...

class ProbabilityConstraint(BaseConstraint, metaclass=abc.ABCMeta):
    def __init__(self, observation: ztyping.NumericalScalarType | ZfitParameter, params: dict[str, ZfitParameter] | None = None, name: str = 'ProbabilityConstraint', dtype=..., **kwargs) -> None: ...
    @property
    def observation(self): ...
    def value(self): ...
    def sample(self, n): ...

class TFProbabilityConstraint(ProbabilityConstraint):
    dist_params: Incomplete
    dist_kwargs: Incomplete
    def __init__(self, observation: ztyping.NumericalScalarType | ZfitParameter, params: dict[str, ZfitParameter], distribution: tfd.Distribution, dist_params, dist_kwargs=None, name: str = 'DistributionConstraint', dtype=..., **kwargs) -> None: ...
    @property
    def distribution(self): ...

class GaussianConstraint(TFProbabilityConstraint, SerializableMixin):
    def __init__(self, params: ztyping.ParamTypeInput, observation: ztyping.NumericalScalarType, *, uncertainty: ztyping.NumericalScalarType = None, sigma: ztyping.NumericalScalarType = None, cov: ztyping.NumericalScalarType = None) -> None: ...
    @property
    def covariance(self): ...

class GaussianConstraintRepr(BaseConstraintRepr):
    hs3_type: Literal['GaussianConstraint']
    params: list[Serializer.types.ParamInputTypeDiscriminated]
    observation: list[Serializer.types.ParamInputTypeDiscriminated]
    uncertainty: list[Serializer.types.ParamInputTypeDiscriminated] | None
    sigma: list[Serializer.types.ParamInputTypeDiscriminated] | None
    cov: list[Serializer.types.ParamInputTypeDiscriminated] | None
    def get_init_args(cls, values): ...
    def validate_params(cls, v): ...

class PoissonConstraint(TFProbabilityConstraint, SerializableMixin):
    def __init__(self, params: ztyping.ParamTypeInput, observation: ztyping.NumericalScalarType) -> None: ...

class PoissonConstraintRepr(BaseConstraintRepr):
    hs3_type: Literal['PoissonConstraint']
    params: list[Serializer.types.ParamInputTypeDiscriminated]
    observation: list[Serializer.types.ParamInputTypeDiscriminated]
    def get_init_args(cls, values): ...
    def validate_params(cls, v): ...

class LogNormalConstraint(TFProbabilityConstraint, SerializableMixin):
    def __init__(self, params: ztyping.ParamTypeInput, observation: ztyping.NumericalScalarType, uncertainty: ztyping.NumericalScalarType) -> None: ...

class LogNormalConstraintRepr(BaseConstraintRepr):
    hs3_type: Literal['LogNormalConstraint']
    params: list[Serializer.types.ParamInputTypeDiscriminated]
    observation: list[Serializer.types.ParamInputTypeDiscriminated]
    uncertainty: list[Serializer.types.ParamInputTypeDiscriminated]
    def get_init_args(cls, values): ...
    def validate_params(cls, v): ...
