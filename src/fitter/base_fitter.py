'''
This module contains BaseFitter
'''
from typing                   import cast

from omegaconf                import OmegaConf, DictConfig
from dmu.stats.fitter         import Fitter
from zfit.result              import FitResult  as zres
from zfit.core.interfaces     import ZfitData   as zdata
from zfit.core.interfaces     import ZfitPDF    as zpdf

# ------------------------
class BaseFitter:
    '''
    Fitting base class, meant to

    - Provide basic functionality to fiters for data and simulation
    - Behave as a dependency sink, avoiding circular imports
    '''
    def __init__(self):
        '''
        '''
