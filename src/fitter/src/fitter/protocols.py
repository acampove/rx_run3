'''
Module holding protocol classes
'''

from typing         import Protocol
from zfit.param     import Parameter as zpar

# -------------------------------------------------------------
class ParameterHolder(Protocol):
    def get_params(self, floating : bool) -> set[zpar]:
        ...
# -------------------------------------------------------------

