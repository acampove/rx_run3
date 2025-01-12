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

from rx_calibration.hltcalibration.parameter import Parameter

log   = LogStore.add_logger('rx_calibration:fitter')
# --------------------------------------------------
class Fitter:
    '''
    Class meant to produce a Parameter object
    from:

    - Simulated and real data stored in ROOT dataframe
    - Signal and background zfit PDFs

    If any of the dataframes does not contain a weights column, one will be defined with ones.
    Otherwise, the weights will be used in the fit.

    The fits to the data will be made by "fixing the tails" to the values in simulation, where
    the tail parameters are the ones whose names do not end with _flt. Usually the mean and
    widths are not considered tails and thus they would be named like `mu_flt` and `sig_flt`.
    '''
    def __init__(self,
                 data : RDataFrame,
                 sim  : RDataFrame,
                 smod : BasePDF,
                 bmod : BasePDF,
                 conf : dict):
        '''
        Parameters
        ------------------
        data : ROOT dataframe with real data
        sim  : ROOT dataframe with simulation
        smod : zfit PDF with signal model
        bmod : zfit PDF with background model
        conf : Dictionary with configuration for fitting, plotting, etc
        '''
        self._rdf_dat = data
        self._rdf_sim = sim
        self._conf    = conf

        self._pdf_sig = smod
        self._pdf_bkg = bmod
        self._pdf_ful : BasePDF
        self._zdt_sig : zdata
        self._zdt_dat : zdata

        self._par_nsg = zfit.Parameter('nsig', 10, 0, 1000_000)
        self._par_nbk = zfit.Parameter('nbkg', 10, 0, 1000_000)

        self._minimizer= zfit.minimize.Minuit()
        self._obs      : ZfitSpace
        self._obs_name : str
    # -------------------------------
    def _initialize(self) -> None:
        log.info('Initializing')
        self._check_extended()

        log.debug('Checking ROOT dataframes')
        self._rdf_sim  = self._check_weights(self._rdf_sim)
        self._rdf_dat  = self._check_weights(self._rdf_dat)

        self._obs      = self._pdf_sig.space
        self._obs_name,= self._pdf_sig.obs

        log.debug('Creating full PDF')
        ebkg           = self._pdf_bkg.create_extended(self._par_nbk)
        esig           = self._pdf_sig.create_extended(self._par_nsg)
        self._pdf_ful  = zfit.pdf.SumPDF([ebkg, esig])

        log.debug('Creating zfit data')
        self._zdt_sig  = self._data_from_rdf(self._rdf_sim)
        self._zdt_dat  = self._data_from_rdf(self._rdf_dat)

        log.info(f'Using observable: {self._obs_name}')
    # -------------------------------
    def _data_from_rdf(self, rdf : RDataFrame) -> zdata:
        weights= self._conf['weights_column']
        d_data = rdf.AsNumpy([self._obs_name, weights])

        arr_obs = d_data[self._obs_name]
        arr_wgt = d_data[weights       ]

        data    = zfit.Data.from_numpy(self._obs, array=arr_obs, weights=arr_wgt)

        return data
    # -------------------------------
    def _check_weights(self, rdf) -> RDataFrame:
        v_col  = rdf.GetColumnNames()
        l_col  = [col.c_str() for col in v_col]

        weights= self._conf['weights_column']
        if weights in l_col:
            log.debug('Weights column {weights} found, not defining ones')
            return rdf

        log.debug('Weights column {weights} not found, defining \"weights\" as ones')
        rdf = rdf.Define('weights', '1')

        return rdf
    # -------------------------------
    def _check_extended(self) -> None:
        if self._pdf_sig.is_extended:
            raise ValueError('Signal PDF should not be extended')

        if self._pdf_bkg.is_extended:
            raise ValueError('Background PDF should not be extended')
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
    def _fit_signal(self) -> Parameter:
        log.info('Fitting signal:')

        print_pdf(self._pdf_sig)

        nll = zfit.loss.UnbinnedNLL(model=self._pdf_sig, data=self._zdt_sig)
        res = self._minimizer.minimize(nll)
        res.hesse(method=self._conf['error_method'])
        par = self._res_to_par(res)

        print(res)
        self._plot_fit(data=self._zdt_sig, model=self._pdf_sig, name = 'fit_sim.png')

        return par
    # -------------------------------
    def _fit_data(self) -> Parameter:
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
    # -------------------------------
    def _fix_tails(self, sig_par : Parameter) -> None:
        s_par = self._pdf_ful.get_params()

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
            par.floating = True

            log.info(f'{name:<20}{"-->":<20}{val:<20.3f}')
    # -------------------------------
    def _plot_fit(self, data : zdata, model : BasePDF, name : str) -> None:
        plot_dir = self._conf['plot_dir']
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
        sig_par = self._fit_signal()
        self._fix_tails(sig_par)

        dat_par = self._fit_data()

        return dat_par
# --------------------------------------------------
