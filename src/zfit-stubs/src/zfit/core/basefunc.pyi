import abc
import zfit
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.exception import ShapeIncompatibleError as ShapeIncompatibleError, SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from .basemodel import BaseModel as BaseModel
from typing import Any
from zfit._interfaces import ZfitFunc as ZfitFunc

class BaseFuncV1(BaseModel, ZfitFunc, metaclass=abc.ABCMeta):
    def __init__(self, obs=None, dtype: type = ..., name: str = 'BaseFunc', params: Any = None) -> None: ...
    def copy(self, **override_params): ...
    def func(self, x: ztyping.XType, name: str = 'value', *, params: ztyping.ParamsTypeInput | None = None) -> ztyping.XType: ...
    def as_pdf(self) -> zfit.interface.ZfitPDF: ...
