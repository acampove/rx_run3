import typing
from _typeshed import Incomplete
from vector._compute.planar import x as x, y as y
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY

def cartesian(lib, xx, xy, yx, yy, x, y): ...
def rhophi(lib, xx, xy, yx, yy, rho, phi): ...

dispatch_map: Incomplete

def dispatch(obj: typing.Any, v: typing.Any) -> typing.Any: ...
