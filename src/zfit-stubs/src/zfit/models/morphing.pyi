from ..core import parameter as parameter
from ..util import ztyping as ztyping
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from ..z.interpolate_spline import interpolate_spline as interpolate_spline
from _typeshed import Incomplete
from collections.abc import Iterable, Mapping
from zfit import z as z
from zfit._interfaces import ZfitBinnedPDF as ZfitBinnedPDF
from zfit.core.binnedpdf import BaseBinnedPDF as BaseBinnedPDF

def spline_interpolator(alpha, alphas, densities): ...

class SplineMorphingPDF(BaseBinnedPDF):
    hists: Incomplete
    alpha: Incomplete
    def __init__(self, alpha: ztyping.ParamTypeInput, hists: Mapping[float | int, Iterable[ZfitBinnedPDF]] | list[ZfitBinnedPDF] | tuple[ZfitBinnedPDF], *, extended: ztyping.ExtendedInputType = None, norm: ztyping.NormInputType = None, name: str | None = 'SplineMorphingPDF', label: str | None = None) -> None: ...
