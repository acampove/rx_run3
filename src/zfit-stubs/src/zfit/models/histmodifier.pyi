from ..core.space import supports as supports
from ..util import ztyping as ztyping
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from .binned_functor import BaseBinnedFunctorPDF as BaseBinnedFunctorPDF
from collections.abc import Mapping
from zfit._interfaces import ZfitBinnedPDF as ZfitBinnedPDF

class BinwiseScaleModifier(BaseBinnedFunctorPDF):
    def __init__(self, pdf: ZfitBinnedPDF, modifiers: bool | Mapping[str, ztyping.ParamTypeInput] | None = None, extended: ztyping.ExtendedInputType = None, norm: ztyping.NormInputType = None, name: str | None = 'BinnedTemplatePDF', label: str | None = None) -> None: ...
