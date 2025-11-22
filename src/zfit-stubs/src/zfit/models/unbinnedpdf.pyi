import pydantic.v1 as pydantic
from ..core.binning import unbinned_to_binindex as unbinned_to_binindex
from ..core.space import supports as supports
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from .functor import BaseFunctor as BaseFunctor
from zfit import z as z
from zfit._interfaces import ZfitSpace as ZfitSpace

class UnbinnedFromBinnedPDF(BaseFunctor):
    def __init__(self, pdf, obs=None, *, extended: ExtendedInputType = None, norm: NormInputType = None, name: str | None = 'UnbinnedFromBinnedPDF', label: str | None = None) -> None: ...

class TypedSplinePDF(pydantic.BaseModel):
    order: None
