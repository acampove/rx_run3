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
    def get_model(self, obs : zobs) -> zpdf:
        '''
        Returns fitting model for data fit
        '''

        mu  = zfit.Parameter('mu', 5200, 4500, 6000)
        sg  = zfit.Parameter('sg',   10,   10, 200)
        pdf = zfit.pdf.Gauss(obs=obs, mu=mu, sigma=sg)

        return pdf
# ------------------------
