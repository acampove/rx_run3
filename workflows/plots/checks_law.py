'''
Script used to orchestrate plots made by different scripts
meant to do checks, e.g. efficiency vs q2
'''
import os
import json
from pathlib               import Path
from functools             import cache

import law
from law.parameter         import Parameter
from omegaconf             import DictConfig, OmegaConf
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_orchestration:plot_2d_law')
# -------------------------------------
class ChecksTask(law.Task):
    config_string : str = Parameter() # type: ignore
    kind          : str = Parameter() # type: ignore
    # -------------------------------------
    def _get_config(self) -> DictConfig:
        '''
        Returns
        --------------------
        Transform JSON string `config_string` with configuration to DictConfig
        and returns it
        '''
        if hasattr(self, '_cfg'):
            return self._cfg

        if not isinstance(self.config_string, str):
            raise ValueError('Configuration string not a string')

        cfg = OmegaConf.create(self.config_string)
        if not isinstance(cfg, DictConfig):
            raise ValueError('cfg not a DictConfig')

        self._cfg = cfg

        return self._cfg
    # -------------------------------------
    def output(self) -> list[law.LocalFileTarget]:
        '''
        Returns
        ---------------
        Name of required output, built from ANADIR and config settings
        '''
        cfg      = self._get_config()
        ANADIR   = os.environ['ANADIR']
        PLTDIR   = Path(ANADIR) / f'plots/checks/{self.kind}'
        if self.kind == 'efficiency_vs_q2':
            dir_path = PLTDIR / f'{cfg.analysis}_{cfg.channel}'
        else:
            raise ValueError(f'Invalid kind: {self.kind}')

        return [ law.LocalFileTarget(dir_path / name) for name in cfg.output ]
    # -------------------------------------
    def run(self):
        '''
        Runs comparison
        '''
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
        from rx_plotter_scripts.plot_2d import main as plot_2d 

        cfg = self._get_config() 

        plot_2d(cfg=cfg.args)
# -------------------------------------
class WrapChecks(law.WrapperTask):
    '''
    This class takes care of steering the workflow
    '''
    # ----------------------
    @cache
    @staticmethod
    def get_settings() -> list[str]:
        '''
        Returns
        --------------
        List of JSON strings defining what each job will do
        '''
        config = gut.load_conf(package='configs', fpath='rx_plots/checks.yaml')
        l_cfg  = [] 

        for kind in ['efficiency_vs_q2']: 
            cfg_kind = config[kind]
            output   = OmegaConf.to_container(cfg_kind.output, resolve=True)
            cfg      = OmegaConf.to_container(cfg_kind.args  , resolve=True)
            json_str = json.dumps({'args' : cfg, 'output' : output}) 
            l_cfg.append(json_str)
    
        return l_cfg 
    # --------------------------
    def requires(self):
        '''
        Defines the sets of tasks in the workflow
        '''
        l_settings = WrapChecks.get_settings()
        for settings in l_settings:
            yield ChecksTask(config_string = settings)
# ----------------------
def main():
    l_settings  = WrapChecks.get_settings()
    nworkers    = min(8, len(l_settings))

    law.run(argv=['WrapChecks', '--workers', str(nworkers), '--log-level', 'INFO'])
# ----------------------
if __name__ == "__main__":
    main()
