'''
Script with tests for FCopy class
'''

from dmu import FCopy 

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test_fcopy')
# ----------------------
def test_simple() -> None:
    '''
    '''
    fcp = FCopy(cfg=cfg)
    fcp.copy(source=source, target=target)
# ----------------------
