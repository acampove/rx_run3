from .. import z as z
from ..core.basepdf import BasePDF as BasePDF
from ..core.parameter import set_values as set_values
from ..core.space import combine_spaces as combine_spaces, convert_to_space as convert_to_space, supports as supports
from ..util.exception import WorkInProgressError as WorkInProgressError
from ..util.warnings import warn_experimental_feature as warn_experimental_feature
from ..util.ztyping import ExtendedInputType as ExtendedInputType, NormInputType as NormInputType
from .functor import BaseFunctor as BaseFunctor
from collections.abc import Mapping
from zfit._interfaces import ZfitIndependentParameter as ZfitIndependentParameter, ZfitPDF as ZfitPDF, ZfitParameter as ZfitParameter, ZfitSpace as ZfitSpace

class ConditionalPDFV1(BaseFunctor):
    @warn_experimental_feature
    def __init__(self, pdf: ZfitPDF, cond: Mapping[ZfitIndependentParameter, ZfitSpace], *, name: str = 'ConditionalPDF', extended: ExtendedInputType = None, norm: NormInputType = None, use_vectorized_map: bool = False, sample_with_replacement: bool = True, label: str | None = None) -> None: ...
    @property
    def cond(self) -> dict[ZfitIndependentParameter, ZfitSpace]: ...
    def copy(self, **override_parameters) -> BasePDF: ...
