'''
Module containing Fitter class
'''
# pylint: disable=import-error, unused-import, too-many-positional-arguments, too-many-arguments

import os
import ROOT
import matplotlib.pyplot as plt
import zfit

from zfit.exception          import ParamNameNotUniqueError
from zfit.core.interfaces    import ZfitSpace
from zfit.core.basepdf       import BasePDF
from zfit.core.data          import Data      as zdata

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from dmu.stats.utilities    import print_pdf
from dmu.stats.zfit_plotter import ZFitPlotter

from rx_calibration.hltcalibration.parameter     import Parameter
from rx_calibration.hltcalibration.fit_component import FitComponent

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
                 rdf        : RDataFrame,
                 components : list[FitComponent],
                 cfg        : dict):
        '''
        Parameters
        ------------------
        data       : ROOT dataframe with real data
        components : List of FitComponent instances
        conf       : Dictionary with configuration for fitting, plotting, etc
        '''
        self._rdf_dat = rdf
        self._l_fcomp = components
        self._conf    = cfg

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
        self._update_conf()

        self._obs      = self._l_pdf[0].space
        self._obs_name,= self._l_pdf[0].obs
        try:
            self._pdf_ful  = zfit.pdf.SumPDF(self._l_pdf)
        except ParamNameNotUniqueError as exc:
            log.error('Non-unique parameter names')
            for pdf in self._l_pdf:
                print_pdf(pdf)

            log.info(exc)
            raise

        log.debug('Creating zfit data')
        self._zdt_dat  = self._data_from_rdf(self._rdf_dat)

        log.info(f'Using observable: {self._obs_name}')
    # -------------------------------
    def _update_conf(self):
        log.debug('Updating plotting configuration')
        d_leg = {}
        for fcomp in self._l_fcomp:
            name_pdf = fcomp.pdf.name
            d_leg[name_pdf] = fcomp.name

        self._conf['plotting']['d_leg'] = d_leg
    # -------------------------------
    def _set_pdfs(self) -> None:
        for fcomp in self._l_fcomp:
            fcomp.run()
            pdf = fcomp.pdf
            name= fcomp.name

            if not isinstance(pdf, BasePDF):
                print(fcomp)
                raise ValueError(f'Could not find PDF for component: {name}')

            if pdf.is_extended:
                raise ValueError(f'PDF for component {name} is extended')

            nevt = zfit.Parameter(f'n{name}', 10, 0, 1000_000)
            pdf.set_yield(nevt)

            log.debug(f'Extracting PDF for component: {name}')

            self._l_pdf.append(pdf)
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
    def _save_pars(self, par : Parameter, name : str) -> None:
        out_dir = self._conf['out_dir' ]
        os.makedirs(out_dir, exist_ok=True)

        pars_path= f'{out_dir}/{name}'
        par.to_json(pars_path)
    # -------------------------------
    def fit(self, skip_fit : bool = False) -> Parameter:
        '''
        Function returning Parameter object holding fitting parameters
        '''
        self._initialize()

        log.info('Fitting data using PDF:')
        print_pdf(self._pdf_ful)

        nll = zfit.loss.ExtendedUnbinnedNLL(model=self._pdf_ful, data=self._zdt_dat)

        if skip_fit:
            return Parameter()

        res = self._minimizer.minimize(nll)

        error_method = self._conf['error_method']
        res.hesse(method=error_method)
        par = self._res_to_par(res)

        print(res)
        self._plot_fit(data=self._zdt_dat, model=self._pdf_ful, name = 'fit_dat.png')
        self._save_pars(par, name='fit_dat.json')

        return par
# --------------------------------------------------
