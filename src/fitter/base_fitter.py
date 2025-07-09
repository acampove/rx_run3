'''
This module contains BaseFitter
'''
from typing                   import cast
import matplotlib.pyplot as plt

from omegaconf                import OmegaConf, DictConfig
from dmu.stats.fitter         import Fitter
from dmu.stats.zfit_plotter   import ZFitPlotter
from dmu.generic              import utilities  as gut
from dmu.stats                import utilities  as sut
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
    def _save_fit(
            self,
            cuts     : dict[str,str],
            cfg      : DictConfig,
            out_path : str,
            model    : zpdf|None,
            res      : zres|None,
            data     : zdata) -> None:
        '''
        Parameters
        --------------
        cuts     : Selection used for fit
        cfg      : Plotting configuration
        out_path : Directory where fit will be saved
        model    : PDF from fit, can be None if dataset was empty
        res      : Zfit result object, can be None if fit was to get a KDE
        data     : data from fit
        '''
        plt_cfg = OmegaConf.to_container(cfg, resolve=True)
        plt_cfg = cast(dict, plt_cfg)

        # If no entries were present
        # There will not be PDF
        if model is not None:
            ptr = ZFitPlotter(data=data, model=model)
            ptr.plot(**plt_cfg)
        else:
            plt.figure()

        sel_path = f'{out_path}/selection.yaml'
        gut.dump_json(cuts, sel_path)

        sut.save_fit(
            data   = data,
            model  = model,
            res    = res,
            fit_dir= out_path)
# ------------------------
