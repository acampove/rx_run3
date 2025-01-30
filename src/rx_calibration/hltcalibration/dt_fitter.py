'''
Module containing Fitter class
'''
# pylint: disable=import-error, unused-import, too-many-positional-arguments, too-many-arguments

import os
import ROOT
import zfit
import matplotlib.pyplot as plt

from zfit.core.interfaces    import ZfitSpace
from zfit.core.basepdf       import BasePDF
from zfit.core.data          import Data      as zdata

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from dmu.stats.utilities    import print_pdf
from dmu.stats.zfit_plotter import ZFitPlotter

from rx_calibration.hltcalibration.parameter     import Parameter
from rx_calibration.hltcalibration.mc_fitter     import MCFitter

log   = LogStore.add_logger('rx_calibration:dt_fitter')
# --------------------------------------------------
class DTFitter:
    '''
    Class meant to produce a Parameter object
    from:

    - data stored in ROOT dataframe
    - List of MCFitter objects
    '''
    # -------------------------------
    def __init__(self,
                 data       : RDataFrame,
                 components : list[MCFitter],
                 conf       : dict):
        '''
        Parameters
        ------------------
        data       : ROOT dataframe with real data
        components : List of MCFitter instances
        conf       : Dictionary with configuration for fitting, plotting, etc
        '''
        self._rdf_dat = data
        self._l_fcomp = components
        self._conf    = conf

        self._l_pdf   : list[BasePDF] = []
        self._pdf_ful : BasePDF
        self._zdt_dat : zdata

        self._minimizer= zfit.minimize.Minuit()
        self._obs      : ZfitSpace
        self._obs_name : str
    # -------------------------------
    def _initialize(self) -> None:
        log.debug('Setting PDFs from fit components')
        self._set_pdfs()

        self._obs      = self._l_pdf[0].space
        self._obs_name,= self._l_pdf[0].obs
        self._pdf_ful  = zfit.pdf.SumPDF(self._l_pdf)

        log.debug('Creating zfit data')
        self._zdt_dat  = self._data_from_rdf(self._rdf_dat)

        log.info(f'Using observable: {self._obs_name}')
    # -------------------------------
    def _set_pdfs(self) -> None:
        for fcomp in self._l_fcomp:
            pdf = fcomp.get_pdf()
            name= fcomp.name
            if pdf.is_extended:
                raise ValueError(f'PDF for component {name} is extended')

            nevt = zfit.Parameter(f'n{name}', 10, 0, 1000_000)
            epdf = pdf.create_extended(nevt)

            log.debug(f'Extracting PDF for component: {name}')

            self._l_pdf.append(epdf)
    # -------------------------------
    def _data_from_rdf(self, rdf : RDataFrame) -> zdata:
        arr_obs = rdf.AsNumpy([self._obs_name])[self._obs_name]
        data    = zfit.Data.from_numpy(self._obs, array=arr_obs)

        return data
    # -------------------------------
    def _res_to_par(self, res : zfit.result.FitResult) -> Parameter:
        error_method = self._conf['error_method']
        if error_method != 'minuit_hesse':
            raise NotImplementedError(f'Method {error_method} not implemented, only minuit_hesse allowed')

        res.freeze()
        obj = Parameter()
        for par_name, d_val in res.params.items():
            val : float = d_val['value']
            err : float = d_val['hesse']['error']

            obj[par_name] = val, err

        return obj
    # -------------------------------
    def _plot_fit(self, data : zdata, model : BasePDF, name : str) -> None:
        plot_dir = self._conf['out_dir' ]
        plot_cfg = self._conf['plotting']

        os.makedirs(plot_dir, exist_ok=True)

        obj   = ZFitPlotter(data=data, model=model)
        obj.plot(**plot_cfg)

        plot_path = f'{plot_dir}/{name}'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)
    # -------------------------------
    def fit(self) -> Parameter:
        '''
        Function returning Parameter object holding fitting parameters
        '''
        self._initialize()

        log.info('Fitting data:')

        print_pdf(self._pdf_ful)

        nll = zfit.loss.ExtendedUnbinnedNLL(model=self._pdf_ful, data=self._zdt_dat)
        res = self._minimizer.minimize(nll)

        error_method = self._conf['error_method']
        res.hesse(method=error_method)
        par = self._res_to_par(res)

        print(res)
        self._plot_fit(data=self._zdt_dat, model=self._pdf_ful, name = 'fit_dat.png')

        return par
# --------------------------------------------------
