
import os
from pathlib import Path
from prefect import flow, task

from dmu.generic                import utilities as gut
from rx_plotter_scripts.compare import main      as compare 
from omegaconf                  import DictConfig, OmegaConf

# Disable GPU
os.environ['CUDA_VISIBLE_DEVICES'] = ''

ANADIR = os.environ['ANADIR']
PLTDIR = Path(ANADIR) / 'plots/comparison/brem_track_2'

@task
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

            cfg            = {}
            cfg['sample' ] = sample
            cfg['trigger'] = trigger
            cfg['q2_bin' ] = q2bin 
            cfg['config' ] = 'resolution'
            cfg['substr' ] = None 
            cfg['brem'   ] = 2
            cfg['block'  ] = block
            cfg['nomva'  ] = True
            cfg['emulate'] = False

            cfg = OmegaConf.create(cfg)

            l_cfg.append(cfg)

    return l_cfg 

@task
def run_compare(settings : DictConfig) -> list[Path]:
    """Run the `compare` command for one set of wildcards."""
    cfg = OmegaConf.to_container(settings, resolve=True)

    plot = compare(settings=cfg)

    return plot

@flow
def main():
    l_settings = _get_settings()

    results = []
    for settings in l_settings:
        l_path = run_compare.submit(settings=settings)
        results.append(l_path)

    return results

if __name__ == "__main__":
    main()
