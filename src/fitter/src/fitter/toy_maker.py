'''
Module holding ToyMaker class
'''
import os
import tqdm
import numpy
import pandas     as pnd
import tensorflow as tf

from pathlib     import Path
from omegaconf   import DictConfig, OmegaConf
from dmu.stats   import utilities  as sut
from dmu.stats   import Constraint
from dmu         import LogStore
from dmu.stats   import Fitter, GofCalculator
from zfit.result import FitResult           as zres
from zfit.loss   import ExtendedUnbinnedNLL as Loss

log=LogStore.add_logger('fitter:toy_maker')
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
        nll   : Loss,
        res   : zres,
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
        if 'out_dir' not in cfg:
            raise ValueError('No "out_dir" key found')

        out_dir  : str  = cfg.out_dir
        out_path : Path = self._ana_dir / out_dir

        log.debug(f'Using output directory: {out_path}')
        out_path.mkdir(parents = True, exist_ok = True)

        cfg.out_dir = out_path

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
        res : zres, 
        gof : tuple[float,int,float],
        itoy: int) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        df: Pandas dataframe with potentially information already in it
        res: FitResult object from last fit
        gof: Tuple with pvalue, nodf and chi2 
        itoy:Index for current fit

        Returns
        -------------
        Input pandas dataframe with extra rows added
        It will add one row per parameter
        '''
        cres  = sut.zres_to_cres(res=res, fall_back_error=-1)
        for name, cfg_par in cres.items():
            nrows = len(df)
            name  = str(name)
            gen   = sut.val_from_zres(res=self._res, name=name)
            df.loc[nrows] = [name, cfg_par.value, cfg_par.error, gen, itoy, gof[0], res.valid]

        return df
    # ----------------------
    def _get_out_path(self) -> Path:
        '''
        Returns
        -------------
        Full path to parquet file where dataframe will be saved
        '''
        out_path = self._cfg.out_dir / 'toys.parquet'
        log.info(f'Saving parameters to: {out_path}')

        return out_path
    # ----------------------
    def _print_parameters(self) -> None:
        '''
        Print likelihood's floating parameters at this moment
        '''
        s_par = self._nll.get_params()

        log.debug(f'{"Parameter":<20}{"Value":<20}')
        log.debug(40 * '-')
        for par in s_par:
            log.debug(f'{par.name:<20}{par.value():<20.3f}')
    # ----------------------
    def get_parameter_information(self) -> pnd.DataFrame:
        '''
        Returns
        ------------
        Pandas dataframe where each row represents a parameter
        '''
        columns = ['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Valid']
        df = pnd.DataFrame(columns=columns)

        self._print_parameters()
        l_sampler = [ model.create_sampler() for model in self._nll.model ]
        nll       = self._nll.create_new(data=l_sampler)

        if nll is None:
            raise ValueError('Failed to create NLL with sampler')

        log.debug('Running toys with config:')
        cfg_str = OmegaConf.to_yaml(self._cfg)
        log.debug('\n' + cfg_str)

        l_total = []
        for itoy in tqdm.tqdm(range(self._cfg.ntoys), ascii=' -'):
            for constraint in self._cns:
                constraint.resample()

            total = 0
            for sampler in l_sampler:
                sampler.resample()
                total += sampler.nentries 
            l_total.append(total)

            with GofCalculator.disabled(value = not self._cfg.run_gof):
                res, gof = Fitter.minimize(nll=nll, cfg=self._cfg.fitting)

            df = self._add_parameters(df=df, res=res, gof=gof, itoy=itoy)

        medn_total = numpy.median(l_total)
        mean_total = numpy.mean(l_total)
        stdv_total = numpy.std(l_total)

        log.info(f'Total yield average: {mean_total:.2f} ± {stdv_total:.2f}')
        log.info(f'Total yield median : {medn_total:.2f} ± {stdv_total:.2f}')

        out_path = self._get_out_path()
        df.to_parquet(out_path)

        return df
# ----------------------
