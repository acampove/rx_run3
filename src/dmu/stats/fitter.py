'''
Module holding zfitter class
'''

import pprint
from typing                   import Union
from functools                import lru_cache

import numpy
import zfit
import pandas as pd

from scipy                    import stats
from zfit.minimizers.strategy import FailMinimizeNaN
from zfit.result              import FitResult
from zfit.core.data           import Data
from dmu.logging.log_store    import LogStore

log = LogStore.add_logger('dmu:statistics:fitter')
#------------------------------
class FitterGofError(Exception):
    '''
    Exception used when GoF cannot be calculated
    '''
#------------------------------
class FitterFailedFit(Exception):
    '''
    Exception used when fitter fails
    '''
#------------------------------
class Fitter:
    '''
    Class meant to be an interface to underlying fitters
    '''
    # pylint: disable=too-many-instance-attributes
    #------------------------------
    def __init__(self, pdf, data):
        self._data_in = data
        self._pdf     = pdf

        self._data_zf : zfit.data.Data
        self._data_np : numpy.ndarray
        self._obs     : zfit.Space
        self._d_par   : dict

        self._ndof           = 10
        self._pval_threshold = 0.01
        self._initialized    = False
    #------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._check_data()

        self._intialized = True
    #------------------------------
    def _check_data(self):
        if   isinstance(self._data_in, numpy.ndarray):
            data_np = self._data_in
        elif isinstance(self._data_in, zfit.Data):
            data_np = zfit.run(zfit.z.unstack_x(self._data_in)) # convert original data to numpy array, needed by _calc_gof
        elif isinstance(self._data_in, pd.DataFrame):
            data_np = self._data_in.to_numpy()
        elif isinstance(self._data_in, pd.Series):
            self._data_in = pd.DataFrame(self._data_in)
            data_np = self._data_in.to_numpy()
        else:
            data_type = str(type(self._data_in))
            raise ValueError(f'Data is not a numpy array, zfit.Data or pandas.DataFrame, but {data_type}')

        data_np       = self._check_numpy_data(data_np)
        self._data_np = data_np
        if not isinstance(self._data_in, zfit.Data):
            self._data_zf = zfit.Data.from_numpy(obs=self._pdf.space, array=data_np)
        else:
            self._data_zf = self._data_in
    #------------------------------
    def _check_numpy_data(self, data):
        shp = data.shape
        if   len(shp) == 1:
            pass
        elif len(shp) == 2:
            _, jval = shp
            if jval != 1:
                log.error(f'Invalid data shape: {shp}')
                raise
        else:
            log.error(f'Invalid data shape: {shp}')
            raise

        ival = data.shape[0]

        data = data[~numpy.isnan(data)]
        data = data[~numpy.isinf(data)]

        fval = data.shape[0]

        if ival != fval:
            log.warning(f'Data was trimmed for inf and nan: {ival} -> {fval}')

        return data
    #------------------------------
    def _bin_pdf(self):
        nbins, min_x, max_x = self._get_binning()
        _, arr_edg = numpy.histogram(self._data_np, bins = nbins, range=(min_x, max_x))

        size = arr_edg.size

        l_bc = []
        for i_edg in range(size - 1):
            low = arr_edg[i_edg + 0]
            hig = arr_edg[i_edg + 1]

            var = self._pdf.integrate(limits = [low, hig])
            val = var.numpy()[0]
            l_bc.append(val * self._data_np.size)

        return numpy.array(l_bc)
    #------------------------------
    def _bin_data(self):
        nbins, min_x, max_x = self._get_binning()
        arr_data, _ = numpy.histogram(self._data_np, bins = nbins, range=(min_x, max_x))
        arr_data    = arr_data.astype(float)

        return arr_data
    #------------------------------
    @lru_cache(maxsize=10)
    def _get_binning(self):
        min_x = numpy.min(self._data_np)
        max_x = numpy.max(self._data_np)
        nbins = self._ndof + self._get_float_pars()

        log.debug(f'Nbins: {nbins}')
        log.debug(f'Range: [{min_x:.3f}, {max_x:.3f}]')

        return nbins, min_x, max_x
    #------------------------------
    def _calc_gof(self):
        log.debug('Calculating GOF')

        arr_data    = self._bin_data()
        arr_modl    = self._bin_pdf()
        norm        = numpy.sum(arr_data) / numpy.sum(arr_modl)
        arr_modl    = norm * arr_modl
        arr_res     = arr_modl - arr_data

        arr_chi2    = numpy.divide(arr_res ** 2, arr_data, out=numpy.zeros_like(arr_data), where=arr_data!=0)
        sum_chi2    = numpy.sum(arr_chi2)
        pvalue      = 1 - stats.chi2.cdf(sum_chi2, self._ndof)

        log.debug(f'{"Data":<20}{"Model":<20}{"chi2":<20}')
        if pvalue < self._pval_threshold:
            for data, modl, chi2 in zip(arr_data, arr_modl, arr_chi2):
                log.debug(f'{data:<20.0f}{modl:<20.3f}{chi2:<20.3f}')

        log.debug(f'Chi2: {sum_chi2:.3f}')
        log.debug(f'Ndof: {self._ndof}')
        log.debug(f'pval: {pvalue:<.3e}')

        return (sum_chi2, self._ndof, pvalue)
    #------------------------------
    def _get_float_pars(self):
        npar     = 0
        s_par    = self._pdf.get_params()
        for par in s_par:
            if par.floating:
                npar+=1

        self._d_par = {par.name : par for par in s_par}

        return npar
    #------------------------------
    def _reshuffle_pdf_pars(self):
        '''
        Will move floating parameters of PDF according
        to uniform PDF
        '''

        s_par = self._pdf.get_params(floating=True)
        for par in s_par:
            ival = par.value()
            fval = numpy.random.uniform(par.lower, par.upper)
            par.set_value(fval)
            log.debug(f'{par.name:<20}{ival:<15.3f}{"->":<10}{fval:<15.3f}{"in":<5}{par.lower:<15.3e}{par.upper:<15.3e}')
    #------------------------------
    def _set_pdf_pars(self, res):
        '''
        Will set the PDF floating parameter values as the result instance
        '''
        l_par_flt = list(self._pdf.get_params(floating= True))
        l_par_fix = list(self._pdf.get_params(floating=False))
        l_par     = l_par_flt + l_par_fix

        d_val = { par.name : dc['value'] for par, dc in res.params.items()}

        log.debug('Setting PDF parameters to best result')
        for par in l_par:
            if par.name not in d_val:
                log.debug(f'Skipping {par.name} = {par.value().numpy():.3e}')
                continue

            val = d_val[par.name]
            log.debug(f'{"":<4}{par.name:<20}{"->":<10}{val:<20.3e}')
            par.set_value(val)
    #------------------------------
    def _get_constraints(self, cfg : dict) -> list[zfit.constraint.GaussianConstraint]:
        '''
        Takes dictionary of constraints as floats, returns list of GaussianConstraint objects
        '''
        if 'constraints' not in cfg:
            log.debug('Not using any constraint')
            return []

        d_const = cfg['constraints']
        s_par   = self._pdf.get_params(floating=True)
        d_par   = { par.name : par for par in s_par}

        log.info('Adding constraints:')
        l_const = []
        for par_name, (par_mu, par_sg) in d_const.items():
            if par_name not in d_par:
                log.error(s_par)
                raise ValueError(f'Parameter {par_name} not found among floating parameters of model, above')

            par = d_par[par_name]

            if par_sg == 0:
                par.floating = False
                log.info(f'{"":<4}{par_name:<15}{par_mu:<15.3e}{par_sg:<15.3e}')
                continue

            const = zfit.constraint.GaussianConstraint(params=par, observation=float(par_mu), uncertainty=float(par_sg))
            log.info(f'{"":<4}{par_name:<25}{par_mu:<15.3e}{par_sg:<15.3e}')
            l_const.append(const)

        return l_const
    #------------------------------
    def _get_ranges(self, cfg : dict) -> list:
        if 'ranges' not in cfg:
            return [None]

        ranges = cfg['ranges']
        log.info('-' * 30)
        log.info(f'{"Low edge":>15}{"High edge":>15}')
        log.info('-' * 30)
        for rng in ranges:
            log.info(f'{rng[0]:>15.3e}{rng[1]:>15.3e}')

        return ranges
    #------------------------------
    def _get_subdataset(self, cfg : dict) -> Data:
        if 'nentries' not in cfg:
            return self._data_zf

        nentries_out = cfg['nentries']
        arr_inp      = self._data_zf.to_numpy().flatten()
        nentries_inp = len(arr_inp)
        if nentries_inp <= nentries_out:
            log.warning(f'Input dataset in smaller than output dataset, {nentries_inp} < {nentries_out}')
            return self._data_zf

        has_weights = self._data_zf.weights is not None

        if has_weights:
            arr_wgt = self._data_zf.weights.numpy()
            arr_inp = numpy.array([arr_inp, arr_wgt]).T

        arr_out = numpy.random.choice(arr_inp, size=nentries_out, replace=False)
        if has_weights:
            arr_out = arr_out.T[0]
            arr_wgt = arr_out.T[1]
        else:
            arr_wgt = None

        data = zfit.data.from_numpy(array=arr_out, weights=arr_wgt, obs=self._data_zf.obs)

        return data
    #------------------------------
    def _get_binned_observable(self, nbins : int):
        obs = self._pdf.space
        [[minx]], [[maxx]] = obs.limits

        binning = zfit.binned.RegularBinning(nbins, minx, maxx, name=obs.label)
        obs_bin = zfit.Space(obs.label, binning=binning)

        return obs_bin
    #------------------------------
    def _get_nbins(self, cfg : dict) -> Union[None, int]:
        if 'likelihood' not in cfg:
            return None

        if 'nbins' not in cfg['likelihood']:
            return None

        return cfg['likelihood']['nbins']
    #------------------------------
    def _get_nll(self, data_zf, constraints, frange, cfg):
        nbins     = self._get_nbins(cfg)
        if nbins is None:
            log.info('No binning was specified, will do unbinned fit')
            pdf = self._pdf
        else:
            log.info(f'Using {nbins} bins for fit')
            obs     = self._get_binned_observable(nbins)
            pdf     = zfit.pdf.BinnedFromUnbinnedPDF(self._pdf, obs)
            data_zf = data_zf.to_binned(obs)

        if not self._pdf.is_extended and nbins is None:
            nll = zfit.loss.UnbinnedNLL(        model=pdf, data=data_zf, constraints=constraints, fit_range=frange)
            return nll

        if     self._pdf.is_extended and nbins is None:
            nll = zfit.loss.ExtendedUnbinnedNLL(model=pdf, data=data_zf, constraints=constraints, fit_range=frange)
            return nll

        if frange is not None:
            raise ValueError('Fit range cannot be defined for binned likelihoods')

        if not self._pdf.is_extended:
            nll = zfit.loss.BinnedNLL(          model=pdf, data=data_zf, constraints=constraints)
            return nll

        if     self._pdf.is_extended:
            nll = zfit.loss.ExtendedBinnedNLL(  model=pdf, data=data_zf, constraints=constraints)
            return nll

        raise ValueError('Likelihood was neither Binned nor Unbinned nor Extended nor non-extended')
    #------------------------------
    def _get_full_nll(self, cfg : dict):
        constraints = self._get_constraints(cfg)
        ranges      = self._get_ranges(cfg)
        data_zf     = self._get_subdataset(cfg)
        l_nll       = [ self._get_nll(data_zf, constraints, frange, cfg) for frange in ranges ]
        nll         = sum(l_nll[1:], l_nll[0])

        return nll
    #------------------------------
    def _print_pars(self, cfg : dict):
        '''
        Will print current values parameters in cfg['print_pars'] list, if present
        '''

        if 'print_pars' not in cfg:
            return

        l_par_name = cfg['print_pars']
        d_par_val  = { name : par.value().numpy() for name, par in self._d_par.items() if name in l_par_name}

        l_name = list(d_par_val.keys())
        l_value= list(d_par_val.values())

        l_form = [   f'{var:<10}' for var in l_name]
        header = ''.join(l_form)

        l_form = [f'{val:<10.3f}' for val in l_value]
        parval = ''.join(l_form)

        log.info(header)
        log.info(parval)
    #------------------------------
    def _minimize(self, nll, cfg : dict) -> tuple[FitResult, tuple]:
        mnm = zfit.minimize.Minuit()
        res = mnm.minimize(nll)

        try:
            gof = self._calc_gof()
        except FitterGofError as exc:
            raise FitterGofError('Cannot calculate GOF') from exc

        chi2, _, pval  = gof
        stat           = res.status

        log.info(f'{chi2:<10.3f}{pval:<10.3e}{stat:<10}')
        self._print_pars(cfg)

        return res, gof
    #------------------------------
    def _fit_retries(self, cfg : dict) -> tuple[dict, FitResult]:
        ntries       = cfg['strategy']['retry']['ntries']
        pvalue_thresh= cfg['strategy']['retry']['pvalue_thresh']
        ignore_status= cfg['strategy']['retry']['ignore_status']

        nll        = self._get_full_nll(cfg = cfg)
        d_pval_res = {}
        last_res   = None
        for i_try in range(ntries):
            try:
                res, gof = self._minimize(nll, cfg)
            except (FailMinimizeNaN, FitterGofError, RuntimeError):
                self._reshuffle_pdf_pars()
                log.warning(f'Fit {i_try:03}/{ntries:03} failed due to exception')
                continue

            last_res = res
            bad_fit  = res.status != 0 or not res.valid

            if not ignore_status and bad_fit:
                self._reshuffle_pdf_pars()
                log.info(f'Fit {i_try:03}/{ntries:03} failed, status/validity: {res.status}/{res.valid}')
                continue

            chi2, _, pval   = gof
            d_pval_res[chi2]=res

            if pval > pvalue_thresh:
                log.info(f'Reached {pval:.3f} (> {pvalue_thresh:.3f}) threshold after {i_try + 1} attempts')
                return {chi2 : res}, res

            self._reshuffle_pdf_pars()

        if last_res is None:
            raise FitterFailedFit('Cannot find any valid fit')

        return d_pval_res, last_res
    #------------------------------
    def _pick_best_fit(self, d_pval_res : dict, last_res : FitResult) -> FitResult:
        nsucc = len(d_pval_res)
        if nsucc == 0:
            log.warning('None of the fits succeeded, returning last result')
            self._set_pdf_pars(last_res)

            return last_res

        l_pval_res= list(d_pval_res.items())
        l_pval_res.sort()
        _, res = l_pval_res[0]

        log.debug('Picking out best fit from {nsucc} fits')
        for chi2, _ in l_pval_res:
            log.debug(f'{chi2:.3f}')

        self._set_pdf_pars(res)

        return res
    #------------------------------
    def _fit_in_steps(self, cfg : dict) -> FitResult:
        l_nsample = cfg['strategy']['steps']['nsteps']
        l_nsigma  = cfg['strategy']['steps']['nsigma']
        l_yield   = cfg['strategy']['steps']['yields']

        res = None
        for nsample, nsigma in zip(l_nsample, l_nsigma):
            log.info(f'Fitting with {nsample} samples')
            cfg_step             = dict(cfg)
            cfg_step['nentries'] = nsample

            nll    = self._get_full_nll(cfg = cfg_step)
            res, _ = self._minimize(nll, cfg_step)
            res.hesse(method='minuit_hesse')
            self._update_par_bounds(res, nsigma=nsigma, yields=l_yield)

        log.info('Fitting full sample')
        nll    = self._get_full_nll(cfg = cfg)
        res, _ = self._minimize(nll, cfg)
        res.hesse(method='minuit_hesse')

        if res is None:
            nsteps = len(l_nsample)
            raise ValueError(f'No fit out of {nsteps} was done')

        return res
    #------------------------------
    def _result_to_value_error(self, res : FitResult) -> dict[str, list[float]]:
        d_par = {}
        for par, d_val in res.params.items():
            try:
                val = d_val['value']
                err = d_val['hesse']['error']
            except KeyError as exc:
                pprint.pprint(d_val)
                raise KeyError(f'Cannot extract value, hesse or error from dictionary above') from exc

            d_par[par.name] = [val, err]

        return d_par
    #------------------------------
    def _update_par_bounds(self, res : FitResult, nsigma : float, yields : list[str]) -> None:
        s_shape_par = self._pdf.get_params(is_yield=False, floating=True)
        d_shp_par   = { par.name : par for par in s_shape_par if par.name not in yields}
        d_fit_par   = self._result_to_value_error(res)

        log.info(60 * '-')
        log.info(f'{"Parameter":<20}{"Low bound":<20}{"High bound":<20}')
        log.info(60 * '-')
        for name, [val, err] in d_fit_par.items():
            if name not in d_shp_par:
                log.debug(f'Skipping {name} parameter')
                continue

            shape       = d_shp_par[name]
            shape.lower = val - nsigma * err
            shape.upper = val + nsigma * err

            log.info(f'{name:<20}{val - err:<20.3e}{val + err:<20.3e}')
    #------------------------------
    def fit(self, cfg : Union[dict, None] = None):
        '''
        Runs the fit using the configuration specified by the cfg dictionary

        Returns fit result
        '''
        self._initialize()

        cfg = {} if cfg is None else cfg

        log.info(f'{"chi2":<10}{"pval":<10}{"stat":<10}')
        if 'strategy' not in cfg:
            nll    = self._get_full_nll(cfg = cfg)
            res, _ = self._minimize(nll, cfg)
            res.hesse(method='minuit_hesse')
        elif 'retry' in cfg['strategy']:
            d_pval_res, last_res = self._fit_retries(cfg)
            res = self._pick_best_fit(d_pval_res, last_res)
        elif 'steps' in cfg['strategy']:
            res = self._fit_in_steps(cfg)
        else:
            raise ValueError('Unsupported fitting strategy')


        return res
#------------------------------
