'''
Module containing DataModel class
'''

from dmu.stats.zfit         import zfit
from dmu.generic            import utilities  as gut

from omegaconf              import DictConfig
from zfit.core.interfaces   import ZfitPDF    as zpdf
from zfit.core.interfaces   import ZfitSpace  as zobs

from fitter.base_model      import BaseModel
from fitter.sim_fitter      import SimFitter

# ------------------------
class DataModel(BaseModel):
    '''
    Model for fitting data samples
    '''
    # ------------------------
    def __init__(self):
        '''

        '''

    # ------------------------
    def get_model(self, obs : zobs) -> zpdf:
        '''
        Returns fitting model for data fit
        '''


        return pdf
# ------------------------
