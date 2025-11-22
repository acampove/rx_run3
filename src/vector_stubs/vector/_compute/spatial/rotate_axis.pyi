import typing
from _typeshed import Incomplete
from vector._compute.planar import x as x, y as y
from vector._compute.spatial import z as z
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ

def cartesian(lib, angle, x1, y1, z1, x2, y2, z2): ...

dispatch_map: Incomplete

def make_conversion(azimuthal1, longitudinal1, azimuthal2, longitudinal2): ...
def dispatch(angle: typing.Any, v1: typing.Any, v2: typing.Any) -> typing.Any: ...
