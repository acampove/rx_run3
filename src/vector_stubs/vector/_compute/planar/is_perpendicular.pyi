import typing
from _typeshed import Incomplete
from vector._compute.planar import dot as dot, rho as rho
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY

dispatch_map: Incomplete

def make_function(azimuthal1, azimuthal2): ...
def dispatch(tolerance: typing.Any, v1: typing.Any, v2: typing.Any) -> typing.Any: ...
