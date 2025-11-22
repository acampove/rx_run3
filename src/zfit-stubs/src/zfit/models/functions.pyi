import abc
from ..core.basefunc import BaseFuncV1 as BaseFuncV1
from ..core.basemodel import SimpleModelSubclassMixin as SimpleModelSubclassMixin
from ..core.space import supports as supports
from ..models.basefunctor import FunctorMixin as FunctorMixin
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from _typeshed import Incomplete
from collections.abc import Callable as Callable, Iterable
from zfit._interfaces import ZfitFunc as ZfitFunc

class SimpleFuncV1(BaseFuncV1):
    def __init__(self, obs: ztyping.ObsTypeInput, func: Callable, name: str = 'Function', **params) -> None: ...

class BaseFunctorFuncV1(FunctorMixin, BaseFuncV1, metaclass=abc.ABCMeta):
    funcs: Incomplete
    def __init__(self, funcs, name: str = 'BaseFunctorFunc', params=None, **kwargs) -> None: ...

class SumFunc(BaseFunctorFuncV1):
    def __init__(self, funcs: Iterable[ZfitFunc], obs: ztyping.ObsTypeInput = None, name: str = 'SumFunc', **kwargs) -> None: ...

class ProdFunc(BaseFunctorFuncV1):
    def __init__(self, funcs: Iterable[ZfitFunc], obs: ztyping.ObsTypeInput = None, name: str = 'SumFunc', **kwargs) -> None: ...

class ZFuncV1(SimpleModelSubclassMixin, BaseFuncV1, metaclass=abc.ABCMeta):
    def __init__(self, obs: ztyping.ObsTypeInput, name: str = 'ZFunc', **params) -> None: ...
    def __init_subclass__(cls, **kwargs) -> None: ...
