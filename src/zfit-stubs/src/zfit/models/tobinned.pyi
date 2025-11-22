from ..core.space import supports as supports
from ..util import ztyping as ztyping
from ..util.warnings import warn_advanced_feature as warn_advanced_feature
from .binned_functor import BaseBinnedFunctorPDF as BaseBinnedFunctorPDF
from _typeshed import Incomplete
from zfit import z as z
from zfit._interfaces import ZfitPDF as ZfitPDF, ZfitSpace as ZfitSpace

class MapNotVectorized(Exception): ...

class BinnedFromUnbinnedPDF(BaseBinnedFunctorPDF):
    pdfs: Incomplete
    def __init__(self, pdf: ZfitPDF, space: ZfitSpace, *, extended: ztyping.ExtendedInputType = None, norm: ztyping.NormInputType = None, name: str | None = None, label: str | None = None) -> None: ...
