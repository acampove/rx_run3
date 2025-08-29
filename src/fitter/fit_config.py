'''
Module containing FitConfig class
'''
import os
import dataclasses
from functools import cached_property

import zfit
from omegaconf              import DictConfig
from dmu.logging.log_store  import LogStore
from zfit                   import Space      as zobs

log=LogStore.add_logger('rx_fitter::fit_config')
# ----------------------
@dataclasses.dataclass
class FitConfig:
    '''
    Class used to store configuration needed for fits
    '''
    fit_cfg : DictConfig
    toy_cfg : DictConfig|None = None

    block   : int  = -1 
    nthread : int  = 1
    q2bin   : str  = ''
    mva_cmb : float= 0.0
    mva_prc : float= 0.0
    log_lvl : int  = 20
    ntoys   : int  = 0
    # ----------------------
    def __post_init__(self):
        '''
        This runs after initialization
        '''
        self._set_logs()
        self._initialize_toy_config()

        if not (0 <= self.mva_cmb < 1):
            raise ValueError(f'Invalid value for combinatorial MVA WP: {self.mva_cmb}')

        if not (0 <= self.mva_prc < 1):
            raise ValueError(f'Invalid value for part reco MVA WP: {self.mva_prc}')
    # ----------------------
    def _initialize_toy_config(self) -> None:
        if self.toy_cfg is None:
            return

        out_dir = f'{self.output_directory}/{self.fit_cfg.output_directory}/{self.q2bin}'
    
        log.info(f'Sending toys to: {out_dir}')
        self.toy_cfg.out_dir = out_dir
    
        log.warning(f'Setting number of toys to: {self.ntoys}')
        self.toy_cfg.ntoys = self.ntoys 
    # ----------------------
    def _set_logs(self) -> None:
        '''
        Will put classes in a given logging level
        '''
        LogStore.set_level('dmu:workflow:cache'                   ,           30)
        LogStore.set_level('dmu:stats:utilities'                  ,           30)
        LogStore.set_level('dmu:stats:model_factory'              ,           30)
        LogStore.set_level('dmu:stats:gofcalculator'              ,           30)
        LogStore.set_level('rx_data:rdf_getter'                   ,           30)
        LogStore.set_level('rx_efficiencies:efficiency_calculator',           30)
        LogStore.set_level('rx_selection:truth_matching'          ,           30)
        LogStore.set_level('rx_selection:selection'               ,           30)
        LogStore.set_level('fitter:prec'                          ,           30)
        LogStore.set_level('dmu:stats:constraint_adder'           , self.log_lvl)
        LogStore.set_level('fitter:prec_scales'                   , self.log_lvl)
        LogStore.set_level('fitter:constraint_reader'             , self.log_lvl)
        LogStore.set_level('fitter:fit_rx_data'                   , self.log_lvl)
    # ----------------------
    @cached_property
    def block_cut(self) -> str:
        '''
        Returns
        -------------
        String used to select block, e.g. `block == 3`
        '''
        if self.block == -1:
            block_cut = 'block == (1)'
        else:
            block_cut =f'block == {self.block}'

        return block_cut
    # ----------------------
    @cached_property
    def fit_name(self) -> str:
        '''
        Builds fit identifier from MVA working points
        '''
        cmb  = int(100 * self.mva_cmb)
        prc  = int(100 * self.mva_prc)
        name = f'{cmb:03d}_{prc:03d}'
    
        return name
    # ----------------------
    @cached_property
    def output_directory(self) -> str:
        '''
        Returns
        -----------------
        This function will return the directory WRT which
        the `output_directory` key in the fit config will be defined
        '''
        if self.block == -1:
            block_name = 'all'
        else:
            block_name = f'b{self.block}'
    
        ana_dir = os.environ.get('ANADIR')
        if ana_dir is None:
            raise RuntimeError('ANADIR variable not set')

        out_dir = f'{ana_dir}/fits/data/{self.fit_name}_{block_name}'
    
        return out_dir
    # ----------------------
    @cached_property
    def observable(self) -> zobs:
        '''
        Returns
        -------------
        Zfit observable
        '''
        cfg_obs      = self.fit_cfg.model.observable[self.q2bin]
        [minx, maxx] = cfg_obs.range
        obs          = zfit.Space(cfg_obs.name, minx, maxx)
    
        return obs
# ----------------------
