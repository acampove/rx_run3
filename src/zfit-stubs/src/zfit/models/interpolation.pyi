from ..core.space import supports as supports
from ..util import ztyping as ztyping
from ..util.exception import SpecificFunctionNotImplemented as SpecificFunctionNotImplemented
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from ..z.interpolate_spline import interpolate_spline as interpolate_spline
from .functor import BaseFunctor as BaseFunctor
from zfit._interfaces import ZfitBinnedPDF as ZfitBinnedPDF

class SplinePDF(BaseFunctor):
    def __init__(self, pdf: ZfitBinnedPDF, order: int | None = None, obs: ztyping.ObsTypeInput = None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str | None = 'SplinePDF', label: str | None = None) -> None: ...
    @property
    def order(self): ...
