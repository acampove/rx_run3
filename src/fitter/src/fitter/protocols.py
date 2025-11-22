'''
Module holding protocol classes
'''

from typing         import Protocol
from zfit.interface import ZfitParameter as zpar

# -------------------------------------------------------------
class ParameterHolder(Protocol):
    def get_params(self, floating : bool) -> set[zpar]:
        ...
# -------------------------------------------------------------

