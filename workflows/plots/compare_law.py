'''
This script is meant to run the orchestration of comparison plots with
Luigi Analysis Workflow
'''
import os
import json
from pathlib                    import Path

import law
from law.parameter              import Parameter
from omegaconf                  import DictConfig, OmegaConf
from dmu.generic                import utilities as gut


class CompareTask(law.Task):
    config_string : str = Parameter()

    def _get_config(self) -> DictConfig:
        if hasattr(self, '_cfg'):
            return self._cfg

        if not isinstance(self.config_string, str):
            raise ValueError('Configuration string not a string')

        cfg = OmegaConf.create(self.config_string)
        if not isinstance(cfg, DictConfig):
            raise ValueError('cfg not a DictConfig')

        self._cfg = cfg

        return self._cfg

    def output(self) -> law.LocalFileTarget:
        cfg    = self._get_config()
        ANADIR = os.environ['ANADIR']
        PLTDIR = Path(ANADIR) / 'plots/comparison/brem_track_2'
        plot   = PLTDIR / f'{cfg.sample}/{cfg.trigger}/{cfg.q2_bin}/{cfg.brem}_{cfg.block}/drop_mva/npv.png'

        return law.LocalFileTarget(plot)

    def run(self):
        """Run the `compare` command for one set of wildcards."""
        from rx_plotter_scripts.compare import main as compare 

        cfg = self._get_config() 
    
        compare(settings=cfg)

class WrapCompare(law.WrapperTask):
    def _get_settings(self) -> list[str]:
        """Return all expected plot paths."""
        config = gut.load_conf(package='configs', fpath='rx_plots/compare.yaml')
    
        l_cfg = [] 
        for args in config['args']:
            sample  = args['sample' ]
            trigger = args['trigger']
            q2bin   = args['q2_bin' ]
    
            for block in [12, 5, 6, 78]:
                if block == 78 and sample == 'Bd_JpsiKst_ee_eq_DPC':
                    continue
    
                for brem in [1, 2]:
                    cfg            = {}
                    cfg['sample' ] = sample
                    cfg['trigger'] = trigger
                    cfg['q2_bin' ] = q2bin 
                    cfg['config' ] = 'resolution'
                    cfg['substr' ] = None 
                    cfg['brem'   ] = brem 
                    cfg['block'  ] = block
                    cfg['nomva'  ] = True
                    cfg['emulate'] = False
    
                    json_str = json.dumps(cfg) 
                    l_cfg.append(json_str)
    
        return l_cfg 
    
    def requires(self):
        l_settings = self._get_settings()
        for settings in l_settings:
            yield CompareTask(config_string = settings)

def main():
    law.run(argv=['WrapCompare', '--workers', '8'])

if __name__ == "__main__":
    main()
