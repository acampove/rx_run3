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
    # ------------------------
    def __init__(self):
        '''

        '''
    # ------------------------
    def _fit(
            self,
            cfg   : DictConfig,
            data  : zdata,
            model : zpdf) -> zres:
        '''
        Parameters
        --------------------
        cfg  : Fitting configuration
        data : Zfit data object
        model: Zfit PDF

        Returns
        --------------------
        DictConfig object with parameters names, values and errors
        '''
        fit_cfg = OmegaConf.to_container(cfg, resolve=True)
        fit_cfg = cast(dict, fit_cfg)

        ftr = Fitter(pdf=model, data=data)
        res = ftr.fit(cfg=fit_cfg)

        return res
# ------------------------
