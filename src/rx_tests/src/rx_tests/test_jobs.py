'''
Module holding TestJobs class
'''

from rx_tests import TestConfig
from rx_tests import PackageConf

# --------------------
class TestJobs:
    '''
    Class meant to provide a dictionary with
    job names mapped to list of commands
    '''
    # --------------------
    def __init__(self, cfg : TestConfig):
        '''
        Parameters
        ---------------
        cfg: Configuration
        '''
        self._cfg = cfg
    # ----------------------
    def _cmd_from_cfg(
        self, 
        group: int,
        cfg  : PackageConf) -> str:
        '''
        Parameters
        -------------
        cfg  : Config for package tests
        group: Index of group

        Returns
        -------------
        String representing basic pytest command
        '''
        return f'pytest {cfg.path} --splits {cfg.splits} --group {group}'
    # --------------------
    def get_jobs(self) -> dict[str,list[str]]:
        '''
        Returns
        -----------------
        dictionary mapping job names to list of
        commands
        '''
        data : dict[str, list[str]] = dict()
        for test, cfg in self._cfg.items():
            data[test] = [ self._cmd_from_cfg(cfg = cfg, group = group)  for group in range(cfg.splits) ]

        return data
# --------------------
