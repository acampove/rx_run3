import numpy as np
from _typeshed import Incomplete
from collections.abc import Callable, Iterable, Mapping
from ordered_set import OrderedSet
from tensorflow.python.types.core import TensorLike
from typing import TypeVar
from uhi.typing.plottable import PlottableHistogram

LowerTypeInput: Incomplete
LowerTypeReturn: Incomplete
UpperTypeInput = LowerTypeInput
UpperTypeReturn = LowerTypeReturn
LowerRectTypeInput: Incomplete
LowerRectTypeReturn: Incomplete
UpperRectTypeInput = LowerTypeInput
UpperRectTypeReturn = LowerTypeReturn
RectLowerReturnType: Incomplete
RectUpperReturnType = RectLowerReturnType
RectLimitsReturnType = tuple[RectLowerReturnType, RectUpperReturnType]
RectLimitsTFReturnType: Incomplete
RectLimitsNPReturnType = tuple[np.ndarray, np.ndarray]
RectLimitsInputType = LowerRectTypeInput | UpperRectTypeInput
LimitsType: Incomplete
LimitsTypeSimpleInput = tuple[float, float] | bool
LimitsTypeInput = tuple[tuple[tuple[float, ...]]] | tuple[float, float] | bool
LimitsTypeReturn = tuple[tuple[tuple[float, ...]], tuple[tuple[float, ...]]] | None | bool
NumericalType = int | float | np.ndarray | TensorLike
LimitsTypeInputV1 = Iterable[NumericalType] | NumericalType | bool | None
LimitsFuncTypeInput = LimitsTypeInput | Callable
LimitsTypeReturn = tuple[np.ndarray, np.ndarray] | None | bool
AxesTypeInput = int | Iterable[int]
AxesTypeReturn = tuple[int] | None
ObsTypeInput: Incomplete
ObsTypeReturn = tuple[str, ...] | None
ObsType = tuple[str]
SpaceOrSpacesTypeInput: Incomplete
SpaceType: str
NormInputType: Incomplete
XType: Incomplete
XTypeInput: Incomplete
XTypeReturnNoData: Incomplete
XTypeReturn: Incomplete
NumericalTypeReturn: Incomplete
DataInputType: Incomplete
BinnedDataInputType = PlottableHistogram | Iterable[PlottableHistogram]
ZfitBinnedDataInputType: Incomplete
AnyDataInputType = DataInputType | BinnedDataInputType
WeightsStrInputType: Incomplete
WeightsInputType: Incomplete
ModelsInputType: Incomplete
PDFInputType: Incomplete
BinnedPDFInputType: Incomplete
BinnedHistPDFInputType = BinnedPDFInputType | PlottableHistogram | Iterable[PlottableHistogram]
FuncInputType: Incomplete
NumericalScalarType: Incomplete
nSamplingTypeIn: Incomplete
ConstraintsTypeInput: Incomplete
ParamsTypeOpt: Incomplete
ParamsNameOpt = str | list[str] | None
ParamsOrNameType = ParamsTypeOpt | Iterable[str] | None
ParameterType = TypeVar('ParameterType', bound=dict[str, 'zfit.core.interfaces.ZfitParameter'])
ParametersType = Iterable[ParameterType]
ParamTypeInput = TypeVar('ParamTypeInput', 'zfit.core.interfaces.ZfitParameter', NumericalScalarType)
ParamsTypeInput: Incomplete
ExtendedInputType = bool | ParamTypeInput | None
BaseObjectType: Incomplete
DependentsType = OrderedSet
CacherOrCachersType: Incomplete
LimitsDictAxes: Incomplete
LimitsDictObs: Incomplete
LimitsDictNoCoords = LimitsDictAxes | LimitsDictObs
LimitsDictWithCoords = dict[str, LimitsDictNoCoords]
BinningTypeInput: Incomplete
OptionsInputType = Mapping[str, object] | None
ConstraintsInputType: Incomplete
ArrayLike: Incomplete
ParamValuesMap: Incomplete
