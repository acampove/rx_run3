from .. import z as z
from ..core.coordinates import convert_to_obs_str as convert_to_obs_str
from ..core.dimension import get_same_obs as get_same_obs
from ..core.parameter import convert_to_parameter as convert_to_parameter
from ..core.space import Space as Space, combine_spaces as combine_spaces
from ..serialization import SpaceRepr as SpaceRepr
from ..serialization.pdfrepr import BasePDFRepr as BasePDFRepr
from ..serialization.serializer import Serializer as Serializer
from ..settings import ztypes as ztypes
from ..util import ztyping as ztyping
from ..util.container import convert_to_container as convert_to_container
from ..util.deprecation import deprecated_norm_range as deprecated_norm_range
from ..util.exception import LimitsIncompatibleError as LimitsIncompatibleError, ModelIncompatibleError as ModelIncompatibleError, NormRangeNotSpecifiedError as NormRangeNotSpecifiedError, ObsIncompatibleError as ObsIncompatibleError
from ..util.warnings import warn_advanced_feature as warn_advanced_feature, warn_changed_feature as warn_changed_feature
from collections.abc import Iterable
from zfit._interfaces import ZfitFunctorMixin as ZfitFunctorMixin, ZfitModel as ZfitModel, ZfitParameter as ZfitParameter, ZfitSpace as ZfitSpace

def extract_daughter_input_obs(obs: ztyping.ObsTypeInput, spaces: Iterable[ZfitSpace]) -> ZfitSpace: ...

class FunctorMixin(ZfitFunctorMixin):
    def __init__(self, models, obs, **kwargs) -> None: ...
    @property
    def models(self) -> list[ZfitModel]: ...
    def get_models(self, names=None) -> list[ZfitModel]: ...

class FunctorPDFRepr(BasePDFRepr):
    pdfs: list[Serializer.types.PDFTypeDiscriminated]
    obs: SpaceRepr | None
    def validate_all_functor(cls, values): ...
