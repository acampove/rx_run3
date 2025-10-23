'''
This script is meant to orchestrate the fits for the resonant mode
'''
import os
import law
import json
import argparse

from pathlib               import Path
from law.parameter         import Parameter
from omegaconf             import DictConfig, OmegaConf
from functools             import cache
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_orchestration:reso_fits_law')
# -------------------------------------
class Fit(law.Task):
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

        self._cfg                     = cfg
        self._cfg['args']['toy_cfg']  = ''
        self._cfg['args']['ntoys'  ]  = 0 
        self._cfg['args']['nthread']  = 1 
        self._cfg['args']['log_lvl']  = 20 

        return self._cfg
    # ----------------------
    def _get_mva_cut(self, args : DictConfig) -> str:
        '''
        Parameters
        -------------
        args: Configuration needed for fit

        Returns
        -------------
        string identifying MVA WP, e.g. 050_050
        '''
        cmb_wp = args.mva_cmb * 100
        prc_wp = args.mva_prc * 100

        return f'{cmb_wp:03.0f}_{prc_wp:03.0f}'
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
        mva_cut = self._get_mva_cut(args=args)
        out_dir = ana_dir / f'fits/data/{mva_cut}_b{args.block}/{args.fit_cfg}/data/{args.q2bin}/brem_000'
        out_dir.mkdir(parents=True, exist_ok=True)

        l_output = [ law.LocalFileTarget(out_dir / name) for name in cfg.outputs ]

        nout = len(l_output)
        log.debug(f'Using {nout} outputs')
        for output in l_output:
            log.debug(output)

        return l_output
    # -------------------------------------
    def run(self):
        '''
        Runs comparison
        '''
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
        from fitter_scripts.fit_rx_reso import main as runner

        cfg = self._get_config() 
        runner(args=cfg.args)
# -------------------------------------
class Fits(law.WrapperTask):
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
        cfg = gut.load_conf(package='configs', fpath='rx_fitter/fits.yaml')

        log.info(20 * '-')
        l_settings = Fits.get_settings(cfg=cfg)
        return [ Fit(config_string = settings) for settings in l_settings ]
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to steer mass scales and resolutions measurements')
    parser.add_argument('-l', '--loglvl' , type=int, help='Logging level', default=20)
    args = parser.parse_args()

    LogStore.set_level('rx_orchestration:reso_fits_law', args.loglvl)
# ----------------------
def main():
    _parse_args()
    law.run(argv=['Fits', '--workers', '8', '--log-level', 'INFO'])
# ----------------------
if __name__ == "__main__":
    main()
