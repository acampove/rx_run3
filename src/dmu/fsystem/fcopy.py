'''
Module holding FCopy class and FCopyConf
'''
import subprocess

from pathlib               import Path
from typing                import Final
from pydantic              import BaseModel
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:fsystem:fcopy')
# ----------------------
class FCopyConf(BaseModel):
    '''
    Class meant to store configuration for FCopy
    '''
    host : str
# ----------------------
class FCopy:
    '''
    Class meant to act as a wrapper to rsync by

    - Checking that the server and local machine are suitable
    - Talking to rsync
    '''
    # ----------------------
    def __init__(self, cfg : FCopyConf):
        '''
        Parameters
        -------------
        cfg: Instance of FCopyConf storing configuration
        '''
        self._cfg : Final[FCopyConf] = cfg
    # ----------------------
    def copy(self, source : Path, target : Path) -> bool:
        '''
        Parameters
        ---------------
        source: Path to file to be copied
        target: Destination path

        Returns
        ---------------
        True if success
        '''
        log.debug('')
        log.debug(source)
        log.debug('--->')
        log.debug(target)

        if self._cfg.server == 'localhost':
            commands = ['rsync', '-a', source, target]
        else:
            raise NotImplementedError(f'Invalid server: {self._cfg.server}')

        subprocess.run(commands)

        return True
# ----------------------
