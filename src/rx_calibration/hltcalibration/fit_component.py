'''
Module holding FitComponent class
'''

import os
import math
from typing import Union


from dmu.generic            import utilities as gut
from dmu.stats              import utilities as sut
from dmu.stats.zfit         import zfit
from dmu.stats.utilities    import print_pdf
from dmu.stats.utilities    import is_pdf_usable
from dmu.stats.zfit_plotter import ZFitPlotter
from dmu.stats.minimizers   import AnealingMinimizer
from dmu.logging.log_store  import LogStore

import numpy
import tensorflow        as tf
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                   import RDataFrame
from zfit.core.data         import Data      as zdata
from zfit.core.interfaces   import ZfitSpace as zobs
from zfit.core.basepdf      import BasePDF   as zpdf
from zfit.result            import FitResult as zres

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
    def __init__(self,
                 cfg      : dict,
                 rdf      : Union[RDataFrame,None],
                 pdf      : Union[zpdf,None],
                 obs      : zobs = None):
        '''
        obs: ZfitSpace instance, not needed unless no PDF was passed
        '''
        if rdf is None and pdf is None:
            raise ValueError('Both PDF and RDataFrame missing')

        self._name      = cfg['name']
        self._fit_cfg   = cfg['fitting' ] if 'fitting'  in cfg else None
        self._out_dir   = cfg['output']['out_dir']

        os.makedirs(self._out_dir, exist_ok=True)

        self._rdf       = rdf
        self._pdf       = pdf
        self._obs       = obs if pdf is None else pdf.space
        self._minx      = float(self._obs.lower)
        self._maxx      = float(self._obs.upper)
        self._obs_name, = self._obs.obs
        self._plt_cfg   = cfg['plotting'] if 'plotting' in cfg else None

        self._yield_value : float
        self._yield_error : float
        self._yield_nentr : int

        self._nentries_dummy_data = 10_000
        self._min_isj_entries     = 300 # Below this threshold we use FFT KDEs 
        self._min_fft_entries     = 100 # Below this threshold we use Exact KDEs 
        self._yield_threshold     = 10

        self._minimizer = self._get_minimizer()
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
    def _add_weights(self, rdf : RDataFrame, wgt_name : str) -> RDataFrame:
        v_col  = rdf.GetColumnNames()
        l_col  = [col.c_str() for col in v_col]

        if wgt_name in l_col:
            log.debug(f'Weights column \"{wgt_name}\" found, not defining ones')
            return rdf

        log.debug(f'Weights column {wgt_name} not found, defining \"weights\" as ones')
        rdf = rdf.Define(wgt_name, '1.0')

        return rdf
    # --------------------
    @gut.timeit
    def _fit(self, zdt : zdata) -> zres:
        log.info(f'Fitting component {self._name}')

        print_pdf(self._pdf)

        nll = zfit.loss.UnbinnedNLL(model=self._pdf, data=zdt)
        res = self._minimizer.minimize(nll)

        return res
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

        log.debug(f'Using {weights_column} name for weights column')

        rdf     = self._add_weights(self._rdf, weights_column)
        d_data  = rdf.AsNumpy([self._obs_name, weights_column])

        arr_obs = d_data[self._obs_name]
        arr_wgt = d_data[weights_column]
        data    = zfit.Data.from_numpy(self._obs, array=arr_obs, weights=arr_wgt)

        arr_wgt_used      = data.weights.numpy() # These are the weights INSIDE the observable range
        self._yield_nentr = len(arr_wgt_used)
        self._yield_value = float(numpy.sum(arr_wgt_used))
        self._yield_error = float(numpy.sqrt(numpy.sum(arr_wgt_used * arr_wgt_used)))

        df = pnd.DataFrame({self._obs_name : arr_obs, weights_column : arr_wgt})

        df.to_json(f'{self._out_dir}/data.json', indent=2)

        return data
    # --------------------
    # TODO: Due to issue in zfit, cannot retrieve label from PDF itself
    # Update this when issue is fixed
    # https://github.com/zfit/zfit/issues/631
    def _plot_fit(self, data : zdata, model : zpdf, label : str = ''):
        if self._plt_cfg is None:
            log.warning('No plotting configuration found, will skip plotting')
            return

        obj=ZFitPlotter(data=data, model=model)
        obj.plot(**self._plt_cfg)

        title = f'Entries={self._yield_value:.0f}'
        if 'title' in self._plt_cfg:
            this_title = self._plt_cfg['title']
            title = f'{title}; {this_title}; {label}'

        obj.axs[0].set_title(title)
        obj.axs[1].set_ylim([-5, +5])
        obj.axs[1].plot([self._minx, self._maxx], [+3, +3], linestyle='--', color='red')
        obj.axs[1].plot([self._minx, self._maxx], [-3, -3], linestyle='--', color='red')

        obj.axs[0].set_yscale('linear')
        plot_path = f'{self._out_dir}/fit_lin.png'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)

        obj.axs[0].set_ylim(bottom=0.1)
        obj.axs[0].set_yscale('log')
        plot_path = f'{self._out_dir}/fit_log.png'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)

        plt.close()
    # -------------------------------
    def _plot_placeholder(self, text : str) -> str:
        if self._plt_cfg is None:
            log.warning('No plotting configuration found, will skip plotting')
            return

        _, ax = plt.subplots()
        ax.text(0.5, 0.5, text, fontsize=20, ha='center', va='center')

        plot_path = f'{self._out_dir}/fit.png'
        log.info(f'Saving fit plot to: {plot_path}')
        plt.savefig(plot_path)
        plt.close()
    # -------------------------------
    def _fix_tails(self, sig_par : Parameter) -> None:
        s_par_flt = self._pdf.get_params(floating= True)
        s_par_fix = self._pdf.get_params(floating=False)
        s_par     = s_par_flt | s_par_fix

        l_par = [ par.name for par in s_par ]
        l_par = sorted(l_par)

        log.debug('Found PDF parameters:')
        for name in l_par:
            log.debug(f'    {name}')

        sig_par = Parameter.remove_suffix(sig_par, '_flt')

        log.debug('Found fixing parameters:')
        for name in sig_par:
            log.debug(f'    {name}')

        log.info(60 * '-')
        log.info('Fixing tails')
        log.info(60 * '-')
        for par in s_par:
            name        = par.name
            is_floating = name.endswith('_flt')
            name        = name.removesuffix('_flt')

            if name not in sig_par:
                log.debug(f'Skipping non signal parameter: {name}')
                continue

            val, _ = sig_par[name]

            par.set_value(val)
            log.info(f'{name:<20}{"-->":<20}{val:<20.3f}')

            if is_floating:
                log.debug(f'Not fixing {name}')
                continue

            par.floating = False
    # --------------------
    def _get_kde_model(self) -> str:
        if 'model' in self._fit_cfg['config'][self._name]:
            name = self._fit_cfg['config'][self._name]['model']
            log.info(f'Using user-defined model {name} for {self._name} KDE')

            return name

        if self._min_isj_entries < self._yield_value:
            log.info(f'Yield above ISJ threshold ({self._yield_value} > {self._min_isj_entries}) picking ISJ')
            return 'ISJ'

        if self._min_fft_entries < self._yield_value < self._min_isj_entries:
            log.info(f'Yield above threshold FFT and below ISJ ({self._min_fft_entries} < {self._yield_value} < {self._min_isj_entries}) picking FFT')
            return 'FFT'

        log.info(f'Yield below FFT threshold ({self._yield_value} < {self._min_fft_entries}) picking Exact')

        return 'Exact'
    # --------------------
    def _get_kde_pdf(self) -> Union[zpdf, None]:
        data = self._get_data()
        if data is None:
            log.warning('No data found, not making KDE')
            return None

        if self._yield_nentr == 0:
            log.warning('No entries found in dataset not building PDF')
            return None

        if self._yield_nentr < self._yield_threshold:
            log.warning(f'Cannot build KDE with {self._yield_nentr}, threshold is {self._yield_threshold}')
            return None

        log.info(f'Building KDE with {self._yield_value:.0f} entries')

        cfg_kde = self._fit_cfg['config'][self._name]['cfg_kde']
        model   = self._get_kde_model()

        # TODO: Improve logic, might need to get PDF callable from function above
        # instead of these lines below
        if   model == 'ISJ':
            log.info('High statistics dataset found => using KDE1DimISJ')
            if 'bandwidth' in cfg_kde:
                del cfg_kde['bandwidth']

            pdf = zfit.pdf.KDE1DimISJ(data, name=self._name, **cfg_kde, label='ISJ')
        elif model == 'FFT':
            log.info('Low statistics dataset found => using KDE1DimFFT')
            pdf = zfit.pdf.KDE1DimFFT(data, name=self._name, **cfg_kde, label='FFT')
        elif model == 'Exact':
            cfg_kde['bandwidth'] = 20
            log.info('Low statistics dataset found => using KDE1DimExact')
            pdf = zfit.pdf.KDE1DimExact(data, name=self._name, **cfg_kde, label='Exact')
        else:
            raise ValueError(f'Invalid KDE model: {model}')

        if not is_pdf_usable(pdf):
            return None

        self._plot_fit(data, pdf, label=model)
        sut.save_fit(
                data   =data,
                model  =pdf,
                res    =None,
                fit_dir=self._out_dir)

        return pdf
    # --------------------
    def _get_data_from_pdf(self):
        if not hasattr(self._pdf, 'arr_wgt') or not hasattr(self._pdf, 'arr_mass'):
            log.debug(f'Not found array of masses and/or weights, making dummy data with {self._nentries_dummy_data}')
            self._yield_value = self._nentries_dummy_data
            self._yield_error = math.sqrt(self._nentries_dummy_data)

            return self._pdf.create_sampler(n=self._nentries_dummy_data)

        arr_wgt           = self._pdf.arr_wgt
        arr_mass          = self._pdf.arr_mass
        self._yield_value = numpy.sum(arr_wgt)
        self._yield_error = math.sqrt(numpy.sum(arr_wgt * arr_wgt))

        log.debug(f'Found array of masses and weights, making real data with {self._yield_value:.0f} entries')
        data = zfit.Data.from_numpy(obs=self._pdf.space, array=arr_mass, weights=arr_wgt)

        return data
    # --------------------
    def get_pdf(self, must_load_pars : bool = False) -> zpdf:
        '''
        Will return PDF

        must_load_pars (bool): If true, it will expect the existence of a JSON file with fitting parameters.
        If False, will do the fit all over again
        '''
        self.run(must_load_pars)

        return self._pdf
    # --------------------
    def run(self, must_load_pars : bool = False) -> Parameter:
        '''
        Will return the PDF

        must_load_pars: Is a flag, if:
            - True : When a dataframe is not passed, the parameters must exist to be loaded
            - False: When a dataframe is not passed, if parameters do not exist will not fit PDF. Used for PDFs that have no MC, e.g. Combinatorial
        '''
        if self._pdf is None:
            log.info('PDF not found, building KDE')
            self._pdf = self._get_kde_pdf()
            if self._pdf is None:
                self._plot_placeholder(text='No entries')

            return Parameter()

        s_par = self._pdf.get_params()
        npar = len(s_par)
        if npar == 0:
            log.info('Found non-parametric PDF returning empty parameters')
            data = self._get_data_from_pdf()
            self._plot_fit(data, self._pdf)
            sut.save_fit(
                    data   =data,
                    model  =self._pdf,
                    res    =None,
                    fit_dir=self._out_dir)

            return Parameter()

        log.info('Parametric PDF found, fitting:')
        if (self._rdf is None) and (not must_load_pars):
            log.info('Dataset not found, returning not fitted PDF')
            data = self._get_data_from_pdf()
            self._plot_fit(data, self._pdf)
            sut.save_fit(
                    data   =data,
                    model  =self._pdf,
                    res    =None,
                    fit_dir=self._out_dir)

            return Parameter()

        pars_path= f'{self._out_dir}/parameters.json'
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
        res = self._fit(data)
        par = self._res_to_par(res)
        self._plot_fit(data, self._pdf)
        sut.save_fit(
                data   =data,
                model  =self._pdf,
                res    =res,
                fit_dir=self._out_dir)

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
def load_fit_component(cfg : dict, pdf : zpdf) -> Union[FitComponent, None]:
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
