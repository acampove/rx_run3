'''
Module holding ToyMaker class
'''
import os
import tqdm
import yaml
import math
import numpy
import pandas     as pnd

from dmu         import LogStore
from dmu.generic import rxran
from dmu.stats   import minimizers, zfit
from dmu.stats   import tensorflow as tf
from dmu.stats   import Constraint
from dmu.stats   import GofCalculator
from dmu.stats   import MinimizerFailError
from dmu.stats   import FitResult
from dmu.stats   import FitConf

from pathlib     import Path
from pydantic    import BaseModel, ConfigDict
from zfit.data   import SamplerData

log  = LogStore.add_logger('fitter:toy_maker')
zlos = zfit.loss.ExtendedUnbinnedNLL | zfit.loss.UnbinnedNLL
zres = zfit.result.FitResult
# ----------------------
class ToyMaker:
    '''
    This class is meant to:

    - Take NLL objects build with zfit
    - Recreate them with toy data
    - Minimize them
    - Collect the results of the fits in a pandas dataframe and return it
    '''
    # ----------------------
    def __init__(
        self,
        nll   : zlos,
        res   : FitResult,
        cns   : list[Constraint],
        cfg   : DictConfig):
        '''
        Parameters
        -------------
        nll  : Zfit negativve log likelihood instance
        res  : Result of actual fit to data. Used to make sure
               toys are generaged with the correct initial parameters
        cfg  : omegaconf dictionary controlling configuration
        cns  : List of constraints, needed for resampling between toys
        '''

        self._ana_dir = Path(os.environ['ANADIR'])

        self._nll   = nll
        self._res   = res
        self._cfg   = self._check_config(cfg=cfg)
        self._cns   = [ cons.calibrate(result = res) for cons in cns ]

        if 'minimizer' in cfg.fitting and cfg.fitting.minimizer == 'context_minimizer':
            self._context_minimizer = True
        else:
            self._context_minimizer = False 

        for cons in self._cns:
            log.debug(cons)

        self._check_gof()
        self._check_gpu()
    # ----------------------
    def _check_config(self, cfg : DictConfig) -> DictConfig:
        '''
        Parameters
        -------------
        cfg: Config passed by user before check

        Returns
        -------------
        Config after:
        - Checks
        - Update of out_dir with full path WRT ANADIR
        '''
        if 'rseed' not in cfg:
            raise ValueError('Missing random seed')

        if 'output' not in cfg:
            raise ValueError('No "output" key found')

        out_path : Path = self._ana_dir / cfg.output 

        log.debug(f'Using output directory: {out_path}')
        out_path.parent.mkdir(parents = True, exist_ok = True)

        cfg.output = out_path

        return cfg
    # ----------------------
    def _check_gpu(self) -> None:
        '''
        Check if using GPU
        '''
        if tf.config.list_physical_devices('GPU'):
            log.warning('Running with GPU')
        else:
            log.debug('Running with CPU')
    # ----------------------
    def _check_gof(self) -> None:
        '''
        Check if GOF setting was specified
        If running GOF show warning
        '''
        if 'run_gof' not in self._cfg:
            raise ValueError('GOF setting not found in config')

        if self._cfg.run_gof:
            log.warning('Running GOF calculation')
            return

        log.debug('Running with GOF disabled')
    # ----------------------
    def _add_parameters(
        self,
        df  : pnd.DataFrame,
        res : FitResult,
        gof : tuple[float,int,float],
        hash: int,
        itoy: int) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        df  : Pandas dataframe with potentially information already in it
        res : FitResult object from last fit
        gof : Tuple with pvalue, nodf and chi2
        hash: Sum of values of masses for all samplers
        itoy: Index for current fit

        Returns
        -------------
        Input pandas dataframe with extra rows added
        It will add one row per parameter
        '''
        pvalue = gof[0]
        for parameter in res.parameters:
            nrows            = len(df)
            name             = parameter.name
            fit_val, fit_err = res[name]
            gen_val, _       = self._res.get(name = name, fall_back = math.nan)
            df.loc[nrows]    = [
                name, 
                fit_val,
                fit_err,
                gen_val, 
                itoy, 
                pvalue, 
                res.valid,
                hash]

        return df
    # ----------------------
    def _print_parameters(self) -> None:
        '''
        Print likelihood's floating parameters at this moment
        '''
        self._print_result(
            msg = 'Reference fit',
            res = self._res)

        s_par = self._nll.get_params()

        log.debug(f'{"Parameter":<20}{"Value":<20}')
        log.debug(40 * '-')
        for par in s_par:
            log.debug(f'{par.name:<20}{par.value():<20.5f}')
    # ----------------------
    def _print_result(self, res : FitResult, msg : str) -> None:
        '''
        Parameters
        -------------
        res: Fit result
        msg: Header message
        '''
        log.debug(30 * '-')
        log.debug(msg)
        log.debug(30 * '-')
        log.debug(res)
        log.debug(30 * '-')
    # ----------------------
    def _resample(
        self, 
        samplers : list[SamplerData]) -> int:
        '''
        Parameters
        -------------
        samplers: List of samplers, i.e. proxies to data

        Returns
        -------------
        Yield of all samplers after resampling
        '''
        log.debug('Resampling constraints')
        for constraint in self._cns:
            constraint.resample()

        sorted_samplers = { self._sampler_name(sampler = sam) : sam for sam in samplers }
        sorted_samplers = dict(sorted(sorted_samplers.items()))

        total = 0
        log.debug(30 * '-')
        log.debug('Resampling samplers')
        log.debug(30 * '-')
        for sampler in sorted_samplers.values():
            old_value  = sampler.n_events
            sampler.resample()
            new_value  = sampler.n_events

            log.debug(f'Yield: {old_value} --> {new_value}')

            total += new_value
        log.debug(30 * '-')

        return total
    # ----------------------
    def _print_yield_stats(self, yields : list[int]) -> None:
        '''
        Parameters
        -------------
        yields: List with number of data entries across toys
        '''
        medn_total = numpy.median(yields)
        mean_total = numpy.mean(yields)
        stdv_total = numpy.std(yields)

        log.info(f'Total yield average: {mean_total:.2f} ± {stdv_total:.2f}')
        log.info(f'Total yield median : {medn_total:.2f} ± {stdv_total:.2f}')
    # ----------------------
    def _sampler_identifier(self, sampler : SamplerData) -> int:
        '''
        Parameters
        -------------
        sampler: Sampler used to run toy fits

        Returns
        -------------
        Number uniquely identifying sampler 
        '''
        val = sampler.numpy().sum()

        return int(val)
    # ----------------------
    def _sampler_name(self, sampler : SamplerData) -> str:
        '''
        Parameters
        -------------
        sampler: Data sampler

        Returns
        -------------
        name associated to sampler, observable's name, for now
        '''
        obs = sampler.obs
        if obs is None:
            raise ValueError('Observable not found in sampler')

        if len(obs) != 1:
            raise ValueError(f'Not found one and only one observable: {sampler.obs}')

        name = obs[0]

        return name
    # ----------------------
    def _sort_samplers(self, samplers : list[SamplerData]) -> list[SamplerData]:
        '''
        Parameters
        -------------
        samplers: List of samplers

        Returns
        -------------
        Same as input, but sorted according to observable

        Requirements
        -------------
        - Same number of samplers as observables
        - Observables have to have different names
        - Names between samplers and models need to match
        '''
        # Only one sampler, trivial
        nsamplers = len(samplers)
        if nsamplers == 1:
            return samplers

        self._print_samplers(samplers = samplers, msg = 'Samplers before sorting')

        # collect name -> sampler
        samplers_dict = { self._sampler_name(sampler = sampler) : sampler for sampler in samplers }

        # collect model names
        name_in_nll  = [ model.obs[0] for model in self._nll.model ]
        unique_names = set(name_in_nll)
        if len(unique_names) < 2:
            raise ValueError(f'Not enough observables found, for {nsamplers} samplers: {unique_names}')

        # check for requirements
        if nsamplers != len(name_in_nll):
            log.error('Observables in NLL')
            for name in name_in_nll:
                log.error(name)

            log.error('Observables in samplers')
            for name in samplers_dict:
                log.error(name)

            raise ValueError('Number of samplers is not the same as number of observables')

        if set(samplers_dict) != set(name_in_nll):
            log.error(f'Samplers: {samplers_dict.keys()}')
            log.error(f'NLL     : {name_in_nll}')
            raise ValueError('Observables in samplers and NLL are different')

        # sort samplers
        sorted_samplers = [ samplers_dict[name] for name in name_in_nll ]

        self._print_samplers(samplers = sorted_samplers, msg = 'Samplers after sorting')

        return sorted_samplers
    # ----------------------
    def _print_samplers(
        self, 
        samplers : list[SamplerData],
        msg      : str | None = None) -> None:
        '''
        Parameters
        -------------
        samplers: Samplers (i.e. proxy to toy data)
        msg     : Message to add to printout
        '''
        log.debug(40 * '-')
        if msg:
            log.debug(msg)
            log.debug(40 * '-')
        log.debug(f'{"Name":<20}{"Size":<20}')
        log.debug(40 * '-')
        for sampler in samplers:
            size = sampler.n_events
            name = self._sampler_name(sampler = sampler) 

            log.debug(f'{name:<20}{size:<20}')
    # ----------------------
    def get_parameter_information(
        self, 
        update   : bool = True,
        samplers : list[SamplerData] | None = None) -> pnd.DataFrame:
        '''
        Parameters
        ------------
        update  : By default True, it will create the file even if already found
        samplers: By default None. If not None, will use these samplers to make toys.

        Returns
        ------------
        Pandas dataframe where each row represents a parameter
        '''
        if not update and os.path.isfile(self._cfg.output):
            log.info(f'Output already found, reusing: {self._cfg.output}')
            return pnd.read_parquet(path = self._cfg.output)

        self._print_parameters()

        if samplers is None:
            log.info('Using model based sampler for toys')
            samplers = [ model.create_sampler() for model in self._nll.model ]
        else:
            log.warning('Using custom sampler for toys')
            samplers = self._sort_samplers(samplers = samplers)

        zfit_cns  = [ cons.zfit_cons(holder = self._nll) for cons in self._cns ]

        self._print_samplers(samplers = samplers, msg = 'Samplers before recreating NLL')
        nll = self._nll.create_new(
            data        = samplers,
            constraints = zfit_cns) # type: ignore
        self._print_samplers(samplers = samplers, msg = 'Samplers after recreating NLL')

        if nll is None:
            raise ValueError('Failed to create NLL with sampler')

        log.debug('Running toys with config:')
        cfg_str = OmegaConf.to_yaml(self._cfg)
        log.debug('\n' + cfg_str)

        l_total = []
        columns = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Valid', 'Hash']
        df      = pnd.DataFrame(columns=columns)

        n_lost  = 0
        for itoy in tqdm.tqdm(range(1, self._cfg.ntoys + 1), ascii=' -'):
            seed = self._cfg.rseed
            with rxran.seed(value = seed, index = itoy):
                log.debug(f'Resampling with: {seed}/{itoy}')
                total = self._resample(samplers = samplers)
                l_total.append(total)

            with GofCalculator.disabled(value = not self._cfg.run_gof):
                try:
                    obj, gof = Fitter.minimize(
                        nll               = nll, 
                        cfg               = self._cfg.fitting,
                        context_minimizer = self._context_minimizer)
                except FitterFailError:
                    n_lost += 1
                    continue

                if isinstance(obj, zres):
                    res = FitResult.from_zfit(res = obj, no_errors_ok = True)
                else:
                    res = obj

                if log.getEffectiveLevel() < 20:
                    self._print_result(res = res, msg = 'Toy fit')

            hashes = [ self._sampler_identifier(sampler = sam) for sam in samplers ]
            hash   = sum(hashes)
            log.debug(30 * '-')
            log.debug(f'Hash: {hash}')
            log.debug(f'Toy : {itoy}')
            log.debug(f'Seed: {self._cfg.rseed}')
            log.debug(30 * '-')

            df = self._add_parameters(
                df  = df,
                res = res,
                gof = gof,
                hash= hash, 
                itoy= itoy)

        if n_lost:
            log.warning(f'Found {n_lost}/{self._cfg.ntoys} failed fits')
        else:
            log.info('No failed fits')

        self._print_yield_stats(yields = l_total)

        log.info(f'Saving to: {self._cfg.output}')
        df = df.sort_values(by = 'Parameter')
        df.to_parquet(self._cfg.output)

        return df
# ----------------------
