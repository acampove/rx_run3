import typing
from _typeshed import Incomplete
from vector._compute.lorentz import dot as dot
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ, TemporalT as TemporalT, TemporalTau as TemporalTau

dispatch_map: Incomplete

def make_function(azimuthal, longitudinal, temporal): ...
def dispatch(tolerance: typing.Any, v: typing.Any) -> typing.Any: ...
