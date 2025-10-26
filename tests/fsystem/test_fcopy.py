'''
Script with tests for FCopy class
'''

from pathlib import Path
from dmu     import FCopy, FCopyConf

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_fcopy')
# ----------------------
def _make_paths(
    dir       : Path, 
    make_file : bool,
    number    : int,
    size      : int = 1) -> list[Path]:
    '''
    Parameters
    -------------
    dir      : Path to directory
    make_file: If true, will make an empty file
    number   : Number of paths to make
    size     : Size in MB, default 1

    Returns
    -------------
    List of paths
    '''
    dir.mkdir(parents=True, exist_ok=True)

    l_path = []
    for index in range(number):
        path = dir / f'{index:03}'

        if make_file:
            with open(path, 'wb') as f:
                f.seek(size * 1024 * 1024 - 1)
                f.write(b'\0')

        l_path.append(path)

    return l_path
# ----------------------
def test_simple(tmp_path) -> None:
    '''
    '''
    l_source = _make_paths(dir=tmp_path / 'source', make_file= True, number=10)
    l_target = _make_paths(dir=tmp_path / 'target', make_file=False, number=10)

    cfg = FCopyConf(host='localhost')
    fcp = FCopy(cfg=cfg)

    for source, target in zip(l_source, l_target):
        fcp.copy(source=source, target=target)
# ----------------------
