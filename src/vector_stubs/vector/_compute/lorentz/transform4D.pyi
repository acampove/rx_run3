import typing
from _typeshed import Incomplete
from vector._compute.lorentz import t as t
from vector._compute.planar import x as x, y as y
from vector._compute.spatial import z as z
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY, LongitudinalEta as LongitudinalEta, LongitudinalTheta as LongitudinalTheta, LongitudinalZ as LongitudinalZ, TemporalT as TemporalT, TemporalTau as TemporalTau

def cartesian_t(lib, xx, xy, xz, xt, yx, yy, yz, yt, zx, zy, zz, zt, tx, ty, tz, tt, x, y, z, t): ...
def cartesian_tau(lib, xx, xy, xz, xt, yx, yy, yz, yt, zx, zy, zz, zt, x, y, z, tau): ...

dispatch_map: Incomplete

def make_conversion(azimuthal, longitudinal, temporal): ...
def dispatch(obj: typing.Any, v: typing.Any) -> typing.Any: ...
