import typing
from _typeshed import Incomplete
from typing import Protocol as Protocol, TypedDict

__all__ = ['BoolCollection', 'FloatArray', 'Protocol', 'ScalarCollection', 'TransformProtocol2D', 'TransformProtocol3D', 'TransformProtocol4D']

ScalarCollection = typing.Any
BoolCollection = typing.Any

class TransformProtocol2D(TypedDict):
    xx: ScalarCollection
    xy: ScalarCollection
    yx: ScalarCollection
    yy: ScalarCollection

class TransformProtocol3D(TypedDict):
    xx: ScalarCollection
    xy: ScalarCollection
    xz: ScalarCollection
    yx: ScalarCollection
    yy: ScalarCollection
    yz: ScalarCollection
    zx: ScalarCollection
    zy: ScalarCollection
    zz: ScalarCollection

class TransformProtocol4D(TypedDict):
    xx: ScalarCollection
    xy: ScalarCollection
    xz: ScalarCollection
    xt: ScalarCollection
    yx: ScalarCollection
    yy: ScalarCollection
    yz: ScalarCollection
    yt: ScalarCollection
    zx: ScalarCollection
    zy: ScalarCollection
    zz: ScalarCollection
    zt: ScalarCollection
    tx: ScalarCollection
    ty: ScalarCollection
    tz: ScalarCollection
    tt: ScalarCollection

FloatArray: Incomplete
