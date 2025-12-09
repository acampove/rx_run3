'''
Module containing FitConfig class
'''
import os
import dataclasses
from functools import cached_property
from pathlib   import Path
from typing    import Any

from dmu.stats.zfit         import zfit
from omegaconf              import DictConfig
from dmu.logging.log_store  import LogStore
from rx_common              import info
from zfit                   import Space      as zobs

log=LogStore.add_logger('rx_fitter::fit_config')
# ----------------------
@dataclasses.dataclass
class FitConfig:
    '''
    Class used to store configuration needed for fits
    '''
    name    : str
    fit_cfg : DictConfig
    toy_cfg : DictConfig|None = None

    block   : int  = -1 
    nthread : int  = 1
    q2bin   : str  = ''
    mva_cmb : float= 0.0
    mva_prc : float= 0.0
    log_lvl : int  = 20
    ntoys   : int  = 0

    overriding_selection : dict[str,str] = dict()
    # ----------------------
    def replace(self, substring : str, value : str) -> None:
        '''
        Parameters
        -------------
        substring: All the fields in the configs will have this substring replaced
        value    : Replacement value for substring
        '''
        fit_cfg = self._replace_in_config(cfg = self.fit_cfg, substring=substring, value=value)
        if not isinstance(fit_cfg, DictConfig):
            raise TypeError('After replacement, a non DictConfig was returned')

        self.fit_cfg = fit_cfg 

        if self.toy_cfg is not None:
            toy_cfg = self._replace_in_config(cfg = self.toy_cfg, substring=substring, value=value)
            if not isinstance(toy_cfg, DictConfig):
                raise TypeError('After replacement, a non DictConfig was returned')

            self.toy_cfg = toy_cfg
    # ----------------------
    def _replace_in_config(
        self, 
        cfg       : Any, 
        substring : str, 
        value     : str) -> Any: 
        '''
        Parameters
        -------------
        cfg      : Config where replacement will happen
        substring: All the fields in the configs will have this substring replaced
        value    : Replacement value for substring

        Returns
        -------------
        Config with replacements
        '''
        if isinstance(cfg, DictConfig):
            for k, v in cfg.items():
                del cfg[k]
                if not isinstance(k, str):
                    raise TypeError('Key of config is not a string: {k}')

                k      = k.replace(substring, value)
                cfg[k] = self._replace_in_config(v, substring=substring, value=value)
            return cfg

        if isinstance(cfg, list):
            return [ self._replace_in_config(cfg=sub_cfg, substring=substring, value=value) for sub_cfg in cfg]

        if isinstance(cfg, str):
            return cfg.replace(substring, value)

        return cfg
    # ----------------------
    def __post_init__(self):
        '''
        This runs after initialization
        '''
        self._set_logs()
        self._initialize_toy_config()

        try:
            self.block = int(self.block)
        except Exception as exc:
            raise TypeError(f'Cannot cast block {self.block} as int') from exc

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
        DEPENDENCIES_LEVEL=30
        TOOL_LEVEL = self.log_lvl

        LogStore.set_level('dmu:workflow:cache'                   , DEPENDENCIES_LEVEL)
        LogStore.set_level('dmu:stats:utilities'                  , DEPENDENCIES_LEVEL)
        LogStore.set_level('dmu:stats:model_factory'              , DEPENDENCIES_LEVEL)
        LogStore.set_level('dmu:stats:gofcalculator'              , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_data:rdf_getter'                   , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_data:path_splitter'                , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_data:sample_emulator'              , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_data:spec_maker'                   , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_efficiencies:efficiency_calculator', DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_selection:truth_matching'          , DEPENDENCIES_LEVEL)
        LogStore.set_level('rx_selection:selection'               , DEPENDENCIES_LEVEL)
        LogStore.set_level('fitter:prec'                          , DEPENDENCIES_LEVEL)
        LogStore.set_level('dmu:stats:constraint_adder'           ,         TOOL_LEVEL)
        LogStore.set_level('fitter:prec_scales'                   ,         TOOL_LEVEL)
        LogStore.set_level('fitter:constraint_reader'             ,         TOOL_LEVEL)
        LogStore.set_level('fitter:fit_rx_data'                   ,         TOOL_LEVEL)
    # ----------------------
    @cached_property
    def mva_cut(self) -> str:
        '''
        Returns
        -------------
        Cut used for MVA
        '''
        return f'(mva_cmb > {self.mva_cmb}) && (mva_prc > {self.mva_prc})'
    # ----------------------
    @cached_property
    def block_cut(self) -> str:
        '''
        Returns
        -------------
        String used to select block, e.g. `block == 3`
        '''
        if self.block == -1:
            return '(1)'

        if self.block == 12:
            return  '(block == 1) || (block == 2)'

        if self.block == 78:
            return  '(block == 7) || (block == 8)'

        if self.block in [1, 2, 3, 4, 5, 6, 7, 8]:
            return f'block == {self.block}'

        raise ValueError(f'Invalid block {self.block}')
    # ----------------------
    @cached_property
    def brem_cut(self) -> str:
        '''
        Returns
        -------------
        Cut on nbrem, normally exclude brem 0 for electron and do nothing for muon
        '''
        if info.is_ee(trigger = self.fit_cfg.trigger):
            return 'nbrem != 0'

        if info.is_mm(trigger = self.fit_cfg.trigger):
            return 'nbrem == 0'

        raise ValueError(f'Invalid trigger: {self.fit_cfg.trigger}')
    # ----------------------
    @cached_property
    def is_electron(self) -> bool:
        '''
        Returns
        -------------
        True if using electron trigger, false otherwise
        '''
        return info.is_ee(self.fit_cfg.trigger)
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
    def output_directory(self) -> Path:
        '''
        Returns
        -----------------
        This function will return the directory WRT which
        the `output_directory` key in the fit config will be defined
        '''
        if not isinstance(self.block, int):
            raise ValueError(f'Block is not an int but {type(self.block)} = {self.block}')

        if self.block == -1:
            block_name = 'all'
        else:
            block_name = f'b{self.block}'
    
        ana_dir = os.environ.get('ANADIR')
        if ana_dir is None:
            raise RuntimeError('ANADIR variable not set')

        out_dir = f'{ana_dir}/fits/data/{self.fit_name}_{block_name}'
    
        return Path(out_dir)
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
