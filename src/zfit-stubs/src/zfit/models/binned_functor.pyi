from .. import z as z
from ..core.binnedpdf import BaseBinnedPDF as BaseBinnedPDF
from ..core.space import supports as supports
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.deprecation import deprecated_norm_range as deprecated_norm_range
from ..util.exception import NormNotImplemented as NormNotImplemented
from ..util.ztyping import BinnedDataInputType as BinnedDataInputType
from .basefunctor import FunctorMixin as FunctorMixin
from _typeshed import Incomplete
from collections.abc import Iterable
from zfit._interfaces import ZfitPDF as ZfitPDF

def preprocess_pdf_or_hist(models: ZfitPDF | Iterable[ZfitPDF] | BinnedDataInputType): ...

class BaseBinnedFunctorPDF(FunctorMixin, BaseBinnedPDF):
    pdfs: Incomplete
    def __init__(self, models, obs, **kwargs) -> None: ...

class BinnedSumPDF(BaseBinnedFunctorPDF):
    pdfs: Incomplete
    def __init__(self, pdfs: ztyping.BinnedHistPDFInputType, fracs: ztyping.ParamTypeInput | None = None, obs: ztyping.ObsTypeInput = None, *, extended: ztyping.ExtendedInputType = None, name: str = 'BinnedSumPDF', label: str | None = None) -> None: ...
