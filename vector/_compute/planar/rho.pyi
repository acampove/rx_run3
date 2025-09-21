import typing
from _typeshed import Incomplete
from vector._compute.planar import rho2 as rho2
from vector._methods import AzimuthalRhoPhi as AzimuthalRhoPhi, AzimuthalXY as AzimuthalXY

def xy(lib, x, y): ...
def rhophi(lib, rho, phi): ...

dispatch_map: Incomplete

def dispatch(v: typing.Any) -> typing.Any: ...
