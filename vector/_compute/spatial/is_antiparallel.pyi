import typing
from _typeshed import Incomplete
from vector._compute.spatial import dot as dot, mag as mag
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ

dispatch_map: Incomplete

def make_function(azimuthal1, longitudinal1, azimuthal2, longitudinal2): ...
def dispatch(tolerance: typing.Any, v1: typing.Any, v2: typing.Any) -> typing.Any: ...
