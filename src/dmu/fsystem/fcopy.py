'''
Module holding FCopy class and FCopyConf
'''
import shutil
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
    source : str = ''
    target : str = ''
# ----------------------
class FCopy:
    '''
    Class meant to act as a wrapper to rsync by

    - Checking that the server and local machine are suitable
    - Talking to rsync
    '''
    # ----------------------
    def __init__(self, cfg : FCopyConf = FCopyConf()):
        '''
        Parameters
        -------------
        cfg: Instance of FCopyConf storing configuration
        '''
        self._cfg : Final[FCopyConf] = cfg
        self._check_utility(name='rsync')
        self._check_utility(name='ssh'  )
        self._check_remote(path=self._cfg.source)
        self._check_remote(path=self._cfg.target)
    # ----------------------
    def _check_remote(self, path : str) -> None:
        '''
        Parameters
        -------------
        path: String representing server, if empty, its local
        '''
        if not path:
            return

        log.debug(f'Checking availability of server: {path}')
    # ----------------------
    def _check_utility(self, name : str) -> None:
        '''
        Check if utility is available

        Parameters
        ----------------
        name: Name of utility, e.g. rsync
        '''
        path = shutil.which(name)
        if path is None:
            raise ValueError(f'Utility \"{name}\" not found')

        log.debug(f'Utility \"{name}\" found at: {path}')
    # ----------------------
    def _get_path(self, path : Path, is_source : bool) -> str:
        '''
        Parameters
        -------------
        path     : Path in server, e.g. /home/user/file.p
        is_source: If true, this is where the file will be copied from

        Returns
        -------------
        String with full path to server or local
        '''
        if not self._cfg.source and not self._cfg.target:
            return str(path)

        if     is_source and not self._cfg.source:
            if not path.exists():
                raise FileNotFoundError(f'Source \"{path}\" missing')

            return str(path)

        if not is_source and not self._cfg.target:
            return str(path)

        if is_source:
            return f'{self._cfg.source}:{path}'
        else:
            return f'{self._cfg.target}:{path}'
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

        spath    = self._get_path(source, is_source= True)
        tpath    = self._get_path(target, is_source=False)
        commands = ['rsync', '-a', spath, tpath]

        subprocess.run(commands)

        return True
# ----------------------
