'''
This module contains the XRootEOS class
'''
from XRootD.client        import FileSystem
from XRootD.client.flags  import DirListFlags, StatInfoFlags

# ----------------------
class XRootEOS:
    '''
    This class is meant to represent an EOS host in order to:

    - List contents
    '''
    # ----------------------
    def __init__(self, host : str) -> None:
        '''
        Parameters
        -------------
        host: Path to EOS instance, e.g. root://eoslhcb.cern.ch
        '''
        self._fs = FileSystem(host)
    # ----------------------
    def glob(self, root : str, ext : str) -> list[str]:
        print(f'Logging into: {root}')
    
        status , dir_list = self._fs.dirlist(root, DirListFlags.STAT)
        if not status.ok:
            raise ValueError(f'Cannot open {root}')
    
        paths   = []
        objects = dir_list.dirlist
        if not objects:
            return paths
    
        for entry in objects:
            is_dir = (entry.statinfo.flags & StatInfoFlags.IS_DIR)
            if is_dir:
                paths += self.glob(root=f'{root}/{entry.name}', ext = ext)
                continue
    
            if entry.name.endswith(f'.{ext}'):
                paths.append(f'{root}/{entry.name}')
    
        return paths
# ----------------------
