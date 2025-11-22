from ..core.binnedpdf import BaseBinnedPDF as BaseBinnedPDF
from ..core.space import supports as supports
from ..util import ztyping as ztyping
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from zfit._interfaces import ZfitBinnedData as ZfitBinnedData

class HistogramPDF(BaseBinnedPDF):
    def __init__(self, data: ztyping.BinnedDataInputType, extended: ztyping.ExtendedInputType | None = None, norm: ztyping.NormInputType | None = None, name: str = 'HistogramPDF', label: str | None = None) -> None: ...
