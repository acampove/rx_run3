from ..core.binnedpdf import BaseBinnedPDF as BaseBinnedPDF
from ..core.space import supports as supports
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented

class BinnedTemplatePDFV1(BaseBinnedPDF):
    def __init__(self, data, sysshape=None, extended=None, norm=None, name: str = 'BinnedTemplatePDF', label: str | None = None) -> None: ...
