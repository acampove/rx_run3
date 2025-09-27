
import os
from pathlib                    import Path
from prefect                    import flow, task
from prefect.task_runners       import ProcessPoolTaskRunner 

from rx_plotter_scripts.compare import main      as compare 
from dmu.generic                import utilities as gut
from omegaconf                  import DictConfig, OmegaConf

ANADIR = os.environ['ANADIR']
PLTDIR = Path(ANADIR) / 'plots/comparison/brem_track_2'

def _get_settings() -> list[DictConfig]:
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

                cfg = OmegaConf.create(cfg)

                l_cfg.append(cfg)

    return l_cfg 

@task
def run_compare(cfg : DictConfig) -> Path:
    """Run the `compare` command for one set of wildcards."""

    plot = PLTDIR / f'{cfg.sample}/{cfg.trigger}/{cfg.q2_bin}/{cfg.brem}_{cfg.block}/drop_mva/npv.png'
    if plot.exists():
        return plot 

    settings = OmegaConf.to_container(cfg, resolve=True)
    if not isinstance(settings, dict):
        raise ValueError('Invalid settings type')

    compare(settings=settings)

    return plot

@flow(task_runner=ProcessPoolTaskRunner(max_workers=10))
def main_task():
    l_settings = _get_settings()
    futures    = [run_compare.submit(cfg=settings) for settings in l_settings]
    _          = [future.result() for future in futures]

if __name__ == "__main__":
    main_task()
