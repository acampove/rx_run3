'''
Script used to orchestrate plots made by the `cutflow` script
'''
import os
import json
from pathlib               import Path

import law
from law.parameter         import Parameter
from omegaconf             import DictConfig, OmegaConf
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_orchestration:cutflow_law')
# -------------------------------------
class CutflowTask(law.Task):
    config_string : str = Parameter() # type: ignore
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
        cfg    = self._get_config()
        ANADIR = os.environ['ANADIR']
        PLTDIR = Path(ANADIR) / f'plots/cutflow/{cfg.args.config}'
        names  = cfg.output[cfg.args.config]

        dir_path = PLTDIR / f'{cfg.args.sample}/{cfg.args.trigger}/{cfg.args.q2bin}'

        return [ law.LocalFileTarget(dir_path / name) for name in names ]
    # -------------------------------------
    def run(self):
        '''
        Runs comparison
        '''
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
        from rx_plotter_scripts.cutflow import main as cutflow 

        cfg = self._get_config() 

        cutflow(settings=cfg.args)
# -------------------------------------
class WrapCutflow(law.WrapperTask):
    '''
    This class takes care of steering the workflow
    '''
    def _get_settings(self) -> list[str]:
        '''
        Returns
        --------------
        List of JSON strings defining what each job will do
        '''
        config = gut.load_conf(package='configs', fpath='rx_plots/cutflow.yaml')
    
        l_cfg = [] 
        for args in config['args']:
            q2bin   = args['q2bin'  ]
            sample  = args['sample' ]
            trigger = args['trigger']
    
            for kind in ['cleanup']:
                cfg             = {}
                cfg['q2bin'   ] = q2bin 
                cfg['sample'  ] = sample
                cfg['trigger' ] = trigger
                cfg['config'  ] = kind 
                cfg['substr'  ] = None 
                cfg['nthreads'] = 1 # Luigi will run in parallel anyway

                output   = OmegaConf.to_container(config.output, resolve=True)
                json_str = json.dumps({'args' : cfg, 'output' : output}) 
                l_cfg.append(json_str)
    
        return l_cfg 
    # --------------------------
    def requires(self):
        '''
        Defines the sets of tasks in the workflow
        '''
        l_settings = self._get_settings()
        for settings in l_settings:
            yield CutflowTask(config_string = settings)
# ----------------------
def main():
    law.run(argv=['WrapCutflow', '--workers', '12'])
# ----------------------
if __name__ == "__main__":
    main()
