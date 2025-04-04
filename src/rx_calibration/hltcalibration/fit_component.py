'''
Module holding FitComponent class
'''

import os
from typing import Union

import numpy
import ROOT
import zfit
import matplotlib.pyplot as plt
import tensorflow        as tf

from ROOT                   import RDataFrame
from zfit.core.data         import Data      as zdata
from zfit.core.basepdf      import BasePDF
from dmu.logging.log_store  import LogStore
from dmu.stats.utilities    import print_pdf
from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.stats.minimizers   import AnealingMinimizer

from rx_calibration.hltcalibration.parameter import Parameter

log   = LogStore.add_logger('rx_calibration:fit_component')
# ----------------------------------------
class FitComponent:
    '''
    Class meant to represent a fitting component

    It will take:

    PDF: To fit to data and fix tails, if not passed, will use KDE
    ROOT dataframe: Modelling the corresponding component in MC, if not passed, will not fit anything and parameters will float
    If the dataframe is passed, it will fit the PDF and fix the parameters whose names do not end with
    `_flt`. It can also plot the fit. The configuration looks like:

    ```yaml
    name    : signal
    out_dir : /tmp/rx_calibration/tests/fit_component
    fitting :
        error_method   : minuit_hesse
        weights_column : weights
    plotting:
        nbins   : 50
        stacked : true
    ```
    '''
    # --------------------
    def __init__(self, cfg : dict, rdf : Union[RDataFrame,None], pdf : Union[BasePDF,None], obs = None):
        '''
        obs: ZfitSpace instance, not needed unless no PDF was passed
        '''
        if rdf is None and pdf is None:
            raise ValueError('Both PDF and RDataFrame missing')

        self._name      = cfg['name']
        self._fit_cfg   = cfg['fitting' ] if 'fitting'  in cfg else None
        self._plt_cfg   = cfg['plotting'] if 'plotting' in cfg else None
        self._out_dir   = cfg['out_dir']

        self._rdf       = rdf
        self._pdf       = pdf
        self._obs       = obs if pdf is None else pdf.space
        self._minx      = float(self._obs.lower)
        self._maxx      = float(self._obs.upper)
        self._obs_name, = self._obs.obs
        self._minimizer = self._get_minimizer()

        self._yield_value : float
        self._yield_error : float
    # --------------------
    def _get_minimizer(self) -> Union[AnealingMinimizer,None]:
        if self._fit_cfg is None:
            return None

        ntries = 10
        if 'ntries' in self._fit_cfg:
            ntries = self._fit_cfg['ntries']

        pvalue = 0.05
        if 'pvalue' in self._fit_cfg:
            pvalue = self._fit_cfg['pvalue']

        minimizer = AnealingMinimizer(ntries=ntries, pvalue=pvalue)

        return minimizer
    # --------------------
    @property
    def name(self) -> str:
        '''
        Returns the name of the fit component
        '''
        return self._name
    # --------------------
    @property
    def pdf(self) -> BasePDF:
        '''
        Returns PDF
        '''
        self.run()

        return self._pdf
    # --------------------
    def _add_weights(self, rdf : RDataFrame, wgt_name : str) -> RDataFrame:
        v_col  = rdf.GetColumnNames()
        l_col  = [col.c_str() for col in v_col]

        if wgt_name in l_col:
            log.debug(f'Weights column {wgt_name} found, not defining ones')
            return rdf

        log.debug(f'Weights column {wgt_name} not found, defining \"weights\" as ones')
        rdf = rdf.Define(wgt_name, '1.0')

        return rdf
    # --------------------
    def _fit(self, zdt : zdata) -> Parameter:
        log.info(f'Fitting component {self._name}')

        print_pdf(self._pdf)

        nll = zfit.loss.UnbinnedNLL(model=self._pdf, data=zdt)
        res = self._minimizer.minimize(nll)

        print(res)
        par = self._res_to_par(res)

        return par
    # -------------------------------
    def _res_to_par(self, res : zfit.result.FitResult) -> Parameter:
        if self._fit_cfg is None:
            raise ValueError('Cannot find fit configuration')

        err_method = self._fit_cfg['error_method']

        if err_method != 'minuit_hesse':
            raise NotImplementedError(f'Method {err_method} not implemented, only minuit_hesse allowed')

        try:
            res.hesse(method=err_method)
        except tf.errors.InvalidArgumentError as exc:
            log.warning('Cannot calculate error, TF issue')
            log.debug(exc)

        res.freeze()
        obj = Parameter()
        for par_name, d_val in res.params.items():
            val : float = d_val['value']
            if 'hesse' not in d_val:
                err = -1
                log.error('Error not found')
            else:
                err : float = d_val['hesse']['error']

            obj[par_name] = val, err

        obj['nSignal'] = self._yield_value, self._yield_error

        return obj
    # -------------------------------
    def _get_data(self) -> zdata:
        if self._fit_cfg is None:
            raise ValueError('Cannot find fit configuration')

        if 'weights_column' not in self._fit_cfg:
            weights_column = 'weights'
        else:
            weights_column = self._fit_cfg['weights_column']

        rdf            = self._add_weights(self._rdf, weights_column)
        d_data         = rdf.AsNumpy([self._obs_name, weights_column])

        arr_obs = d_data[self._obs_name]
        arr_wgt = d_data[weights_column]
        data    = zfit.Data.from_numpy(self._obs, array=arr_obs, weights=arr_wgt)

        self._yield_value = float(numpy.sum(arr_wgt))
        self._yield_error = float(numpy.sqrt(numpy.sum(arr_wgt * arr_wgt)))

        return data
    # --------------------
    def _plot_fit(self, data : zdata, model : BasePDF):
        if self._plt_cfg is None:
            log.warning('No plotting configuration found, will skip plotting')
            return

        os.makedirs(self._out_dir, exist_ok=True)

        obj=ZFitPlotter(data=data, model=model)
        obj.plot(**self._plt_cfg)

        arr_val = data.to_numpy()
        nentries= len(arr_val)
        title   = f'Entries={nentries}'

        obj.axs[0].set_title(title)
        obj.axs[0].set_ylim(bottom=0)
        obj.axs[1].set_ylim([-5, +5])
        obj.axs[1].plot([self._minx, self._maxx], [+3, +3], linestyle='--', color='red')
        obj.axs[1].plot([self._minx, self._maxx], [-3, -3], linestyle='--', color='red')

        plot_path = f'{self._out_dir}/fit.png'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)
        plt.close()
    # -------------------------------
    def _plot_placeholder(self, text : str) -> str:
        if self._plt_cfg is None:
            log.warning('No plotting configuration found, will skip plotting')
            return

        os.makedirs(self._out_dir, exist_ok=True)

        _, ax = plt.subplots()
        ax.text(0.5, 0.5, text, fontsize=20, ha='center', va='center')

        plot_path = f'{self._out_dir}/fit.png'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)
        plt.close()
    # -------------------------------
    def _fix_tails(self, sig_par : Parameter) -> None:
        s_par = self._pdf.get_params()

        log.debug('Found PDF parameters:')
        for par in s_par:
            log.debug(f'    {par.name}')

        log.debug('Found fixing parameters:')
        for name in sig_par:
            log.debug(f'    {name}')

        log.info(60 * '-')
        log.info('Fixing tails')
        log.info(60 * '-')
        for par in s_par:
            name = par.name
            if name not in sig_par:
                log.debug(f'Skipping non signal parameter: {name}')
                continue

            if name.endswith('_flt'):
                log.debug(f'Not fixing {name}')
                continue

            val, _ = sig_par[name]

            par.set_value(val)
            par.floating = False

            log.info(f'{name:<20}{"-->":<20}{val:<20.3f}')
    # --------------------
    def _get_kde_pdf(self) -> Union[BasePDF, None]:
        data    = self._get_data()
        arr_val = data.to_numpy()
        nentries= len(arr_val)
        if nentries == 0:
            log.warning('No entries found, cannot build KDE')
            self._plot_placeholder(text='No entries')
            return None

        cfg_kde = self._fit_cfg['config'][self._name]['cfg_kde']
        pdf     = zfit.pdf.KDE1DimFFT(data, **cfg_kde)

        self._plot_fit(data, pdf)

        return pdf
    # --------------------
    def _get_data_from_pdf(self):
        if not hasattr(self._pdf, 'arr_wgt') or not hasattr(self._pdf, 'arr_mass'):
            return self._pdf.create_sampler(n=10_000)

        data = zfit.Data.from_numpy(obs=self._pdf.space, array=self._pdf.arr_mass, weights=self._pdf.arr_wgt)

        return data
    # --------------------
    def run(self, must_load_pars : bool = False) -> Parameter:
        '''
        Will return the PDF

        must_load_pars: Is a flag, if:
            - True : When a dataframe is not passed, the parameters must exist to be loaded
            - False: When a dataframe is not passed, if parameters do not exist will not fit PDF. Used for PDFs that have no MC, e.g. Combinatorial
        '''
        pars_path= f'{self._out_dir}/fit.json'

        if self._pdf is None:
            log.info('PDF not found, building KDE')
            self._pdf = self._get_kde_pdf()
            return Parameter()

        s_par = self._pdf.get_params()
        npar = len(s_par)
        if npar == 0:
            log.info('Found non-parametric PDF returning empty parameters')
            data = self._get_data_from_pdf()
            self._plot_fit(data, self._pdf)
            return Parameter()

        log.info('Parametric PDF found, fitting:')
        if self._rdf is None and not must_load_pars:
            log.info('Dataset not found, returning not fitted PDF')
            data = self._get_data_from_pdf()

            return Parameter()

        if os.path.isfile(pars_path):
            log.info(f'Fit parameters for component {self._name} found, loading: {pars_path}')
            par = Parameter.from_json(pars_path)
            self._fix_tails(par)

            return par

        log.debug(f'Fit parameters for component {self._name} not found, missed: {pars_path}')
        if self._rdf is None and must_load_pars:
            log.debug('No data found and data is expected')
            raise NoFitDataFound

        data=self._get_data()
        par = self._fit(data)
        self._plot_fit(data, self._pdf)
        par.to_json(pars_path)

        self._fix_tails(par)

        return par
# ----------------------------------------
class NoFitDataFound(Exception):
    '''
    Meant to be used if a FitComponent is requested but dataframe is None 
    '''
    def __init__(self, message='No ROOT dataframe provided'):
        self.message = message
        super().__init__(self.message)
# ----------------------------------------
def load_fit_component(cfg : dict, pdf : BasePDF) -> Union[FitComponent, None]:
    '''
    Will return a FitComponent instance, if parameters found
    otherwise will return None
    '''
    obj = FitComponent(cfg=cfg, pdf=pdf, rdf=None)
    try:
        obj.run(must_load_pars = True)
    except NoFitDataFound:
        return None

    return obj
# ----------------------------------------
