import typing
from _typeshed import Incomplete
from vector._compute.lorentz import t as t
from vector._compute.spatial import dot as dot
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ, TemporalT as TemporalT, TemporalTau as TemporalTau

dispatch_map: Incomplete

def make_conversion(azimuthal1, longitudinal1, temporal1, azimuthal2, longitudinal2, temporal2): ...
def dispatch(v1: typing.Any, v2: typing.Any) -> typing.Any: ...
