import typing
from _typeshed import Incomplete
from vector._compute.planar import x as x, y as y
from vector._compute.spatial import z as z
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ

def cartesian(lib, u, i, j, k, x, y, z): ...

dispatch_map: Incomplete

def make_conversion(azimuthal, longitudinal): ...
def dispatch(u: typing.Any, i: typing.Any, j: typing.Any, k: typing.Any, vec: typing.Any) -> typing.Any: ...
