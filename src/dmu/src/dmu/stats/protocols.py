'''
Module holding statistics related protocol classes
'''
from typing                import Protocol
from zfit.core.loss        import ZfitParameter

# ----------------------------------------
class ParsHolder(Protocol):
    '''
    Class meant to symbolize generic holder of parameters
    '''
    def get_params(self, *args, **kwargs)-> set[zpar] | set[ZfitParameter]:
        ...

