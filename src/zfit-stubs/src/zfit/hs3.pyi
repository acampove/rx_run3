from .core.serialmixin import ZfitSerializable
from .util.warnings import warn_experimental_feature

__all__ = ['dumps', 'loads']

@warn_experimental_feature
def dumps(obj: ZfitSerializable) -> str: ...
@warn_experimental_feature
def loads(string: str) -> ZfitSerializable: ...
