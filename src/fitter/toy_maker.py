'''
Module holding ToyMaker class
'''
import os
import tqdm
import pandas     as pnd
import tensorflow as tf

from omegaconf                  import DictConfig
from zfit.minimizers.interface  import ZfitResult as zres
from dmu.stats                  import utilities  as sut
from dmu.logging.log_store      import LogStore
from dmu.stats.fitter           import Fitter, GofCalculator
from dmu.stats.constraint_adder import ConstraintAdder
from zfit.loss                  import ExtendedUnbinnedNLL as Loss

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
        cfg   : DictConfig):
        '''
        Parameters
        -------------
        nll  : Zfit negativve log likelihood instance
        res  : Result of actual fit to data. Used to make sure
               toys are generaged with the correct initial parameters
        cfg  : omegaconf dictionary controlling configuration
        '''
        self._nll   = nll
        self._res   = res
        self._cfg   = self._check_config(cfg=cfg) 

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
        Config after check, if checks passed
        '''
        if 'out_dir' not in cfg:
            raise ValueError('No "out_dir" key found')

        out_dir = cfg.out_dir
        log.debug(f'Using output directory: {out_dir}')
        os.makedirs(out_dir, exist_ok=True)

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
            df.loc[nrows] = [name, cfg_par.value, cfg_par.error, gen, itoy, gof[0], res.converged]

        return df
    # ----------------------
    def _get_out_path(self) -> str:
        '''
        Returns
        -------------
        Full path to parquet file where dataframe will be saved
        '''
        out_path = f'{self._cfg.out_dir}/toys.parquet'
        log.info(f'Saving parameters to: {out_path}')

        return out_path
    # ----------------------
    def get_parameter_information(self) -> pnd.DataFrame:
        '''
        Returns
        ------------
        Pandas dataframe where each row represents a parameter
        '''
        df = pnd.DataFrame(columns=['Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Converged'])

        l_sampler = [ model.create_sampler() for model in self._nll.model ]
        nll       = self._nll.create_new(data=l_sampler)
        if nll is None:
            raise ValueError('Faled to create NLL with sampler')

        if 'constraints' not in self._cfg:
            log.warning('Running toys without constraints on any model parameter')
        else:
            log.debug('Using constraints in toy fitting model')

        for itoy in tqdm.tqdm(range(self._cfg.ntoys), ascii=' -'):
            for sampler in l_sampler:
                sampler.resample()

            if 'constraints' in self._cfg:
                cad = ConstraintAdder(nll=nll, cns=self._cfg.constraints)
                nll = cad.get_nll(mode='toy')

            with GofCalculator.disabled(value = not self._cfg.run_gof):
                res, gof = Fitter.minimize(nll=nll, cfg=self._cfg.fitting)

            df = self._add_parameters(df=df, res=res, gof=gof, itoy=itoy)

        out_path = self._get_out_path()
        df.to_parquet(out_path)

        return df
# ----------------------
