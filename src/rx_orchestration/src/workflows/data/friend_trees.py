'''
This module contains code needed to steer the creation of the friend trees
'''
import os
import re
import law
import json
import argparse

from pathlib        import Path
from law.parameter  import Parameter
from dmu.generic    import utilities as gut
from dmu            import LogStore
from omegaconf      import DictConfig, OmegaConf
from rx_data        import NtuplePartitioner

log=LogStore.add_logger('rx_orchestration:friend_trees')
# -------------------------------------
class Friend(law.Task):
    config_string : str = Parameter() # type: ignore
    index         : int = Parameter() # type: ignore
    # ----------------------
    def _get_args(self) -> DictConfig:
        '''
        This method builds the arguments for the runner 
        '''
        data = json.loads(s=self.config_string)

        config = {
            'kind'  : data['kind'   ],
            'proj'  : data['project'],
            'vers'  : data['version'],
            'igroup': self.index,
            'ngroup': data['parts']
        }

        return OmegaConf.create(config)
    # ----------------------
    def _get_friend_path(
        self, 
        path : Path,
        conf : DictConfig) -> law.LocalFileTarget:
        '''
        Parameters
        -------------
        path: Path to file from the main category to be processes
        conf: Dictionary with configuration

        Returns
        -------------
        File object, corresponding to file to be made
        '''
        kind  = conf['kind']
        vers  = conf['version']
        spath = str(path)
        spath = spath.replace('/main/', f'/{kind}/')
        spath = re.sub(
            pattern = r'/v\d+(\.\d+)?/', 
            repl    = f'/{vers}/', 
            string  = spath) 

        log.debug(spath)

        return law.LocalFileTarget(spath)
    # -------------------------------------
    def output(self) -> list[law.LocalFileTarget]:
        '''
        Returns
        ---------------
        List of objects symbolizing files that have to be created
        '''
        conf= json.loads(s=self.config_string)
        prt = NtuplePartitioner(
            kind    = 'main', 
            project = conf['project'])

        paths   = prt.get_paths(index=self.index, total=conf['parts'])
        l_fpath = []
        for path in paths:
            fpath = self._get_friend_path(path = path, conf = conf)
            l_fpath.append(fpath)

        return l_fpath
    # -------------------------------------
    def run(self):
        '''
        Runs creation of friend tree
        '''
        os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
        from rx_data_scripts.branch_calculator import main as runner

        args = self._get_args()

        runner(args=args)
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
        data = gut.load_data(package='configs', fpath='rx_data/friends.yaml')

        log.info(20 * '-')
        tasks = []
        for conf in data['stage_1']:
            config_string = json.dumps(conf)
            ngroups       = conf['parts']
            for index in range(ngroups):
                task          = Friend(config_string = config_string, index = index)
                tasks.append(task)

        return tasks
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
