'''
Module containing DataModel class
'''

from dmu.stats.zfit         import zfit
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
    def get_model(self) -> zpdf:
        '''
        Returns fitting model for data fit
        '''
        pdf = sut.get_model(name='s+b')

        return pdf
# ------------------------
