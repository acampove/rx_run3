'''
This module contains code needed to steer the creation of the friend trees
'''
import os
import law
import argparse

from law.parameter  import Parameter
from pathlib        import Path
from dmu.generic    import utilities as gut
from dmu            import LogStore

log=LogStore.add_logger('rx_orchestration:friend_trees')
# -------------------------------------
class Friend(law.Task):
    config_string : str = Parameter() # type: ignore
    # -------------------------------------
    def output(self) -> list[law.LocalFileTarget]:
        pass
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
class Friends(law.WrapperTask):
    '''
    This class takes care of steering the workflow
    '''
    # --------------------------
    def requires(self):
        '''
        Defines the sets of tasks in the workflow
        '''
        cfg = gut.load_conf(package='configs', fpath='rx_data/friends.yaml')

        log.info(20 * '-')
        l_settings = Friends.get_settings(cfg=cfg)

        return [ Friend(config_string = settings) for settings in l_settings ]
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script use to create friend trees')
    parser.add_argument('-l', '--loglvl' , type=int, help='Logging level', default=20)
    args = parser.parse_args()

    LogStore.set_level('rx_orchestration:friend_trees', args.loglvl)
# ----------------------
def main():
    _parse_args()
    law.run(argv=['Friends', '--workers', '8', '--log-level', 'INFO'])
# ----------------------
if __name__ == "__main__":
    main()
