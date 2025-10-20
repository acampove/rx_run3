'''
This script is meant to steer the calculation of mass scales and resolutions 
'''
import os
import json
import argparse
from functools             import cache
from pathlib               import Path

import law
from law.parameter         import Parameter
from omegaconf             import DictConfig, OmegaConf
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_orchestration:q2_scales')
# -------------------------------------
class FitTask(law.Task):
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

        self._cfg                     = cfg
        self._cfg['args']['kind']     = self.kind 
        self._cfg['args']['syst']     = 'nom' 
        self._cfg['args']['logl']     = 20 
        self._cfg['args']['skip_fit'] = False 

        return self._cfg
    # -------------------------------------
    def output(self) -> list[law.LocalFileTarget]:
        '''
        Returns
        ---------------
        Name of required output, built from ANADIR and config settings
        '''
        cfg     = self._get_config()
        args    = cfg.args

        ana_dir = Path(os.environ['ANADIR'])

        path_1  = f'q2/fits/{args.vers}/{args.kind}'
        path_2  = f'{args.proj}_{args.year}'
        path_3  = f'{args.brem}_{args.block}_{args.syst}'

        out_dir = ana_dir / path_1 / path_2 / path_3
        out_dir.mkdir(parents=True, exist_ok=True)

        l_output = [ law.LocalFileTarget(out_dir / name) for name in cfg.outputs ]

        nout = len(l_output)
        log.info(f'Using {nout} outputs')
        for output in l_output:
            log.debug(output)

        return l_output
    # -------------------------------------
    def requires(self):
        '''
        Defines the sets of tasks in the workflow
        '''
        if self.kind == 'dat':
            return FitTask(config_string = self.config_string, kind='sim')
    # -------------------------------------
    def run(self):
        '''
        Runs comparison
        '''
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
        from rx_q2_scripts.get_q2_tables import main as runner

        cfg = self._get_config() 
        runner(cfg=cfg.args)
# -------------------------------------
class WrapFits(law.WrapperTask):
    '''
    This class takes care of steering the workflow
    '''
    # ----------------------
    @cache
    @staticmethod
    def get_settings(cfg : DictConfig) -> list[str]:
        '''
        Parameters
        --------------
        cfg : Dictionary with configuration

        Returns
        --------------
        List of JSON strings defining what each job will do
        '''
        l_cfg    = [] 
        outputs  = OmegaConf.to_container(cfg.outputs, resolve=True)

        for args in cfg.args:
            args     = OmegaConf.to_container(args, resolve=True)
            json_str = json.dumps({'args' : args, 'outputs' : outputs}) 
            l_cfg.append(json_str)
    
        return l_cfg 
    # --------------------------
    def requires(self):
        '''
        Defines the sets of tasks in the workflow
        '''
        cfg = gut.load_conf(package='configs', fpath='rx_q2/scales.yaml')

        log.info(20 * '-')
        l_settings = WrapFits.get_settings(cfg=cfg)
        return [ FitTask(config_string = settings, kind='dat') for settings in l_settings ]
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to steer mass scales and resolutions measurements')
    parser.add_argument('-l', '--loglvl' , type=int, help='Logging level', default=20)
    args = parser.parse_args()

    LogStore.set_level('rx_orchestration:q2_scales', args.loglvl)
# ----------------------
def main():
    _parse_args()
    law.run(argv=['WrapFits', '--workers', '8', '--log-level', 'INFO'])
# ----------------------
if __name__ == "__main__":
    main()
