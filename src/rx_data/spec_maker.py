'''
Module with SpecMaker class
'''
from pathlib               import Path
from rx_common.types       import Trigger
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:spec_maker')
# --------------------------
class SpecMaker:
    '''
    Class meant to:

    - Find samples and use them to create a JSON file with them
    - Save file and make path available to user
    '''
    # ----------------------
    def __init__(self, sample : str, trigger : Trigger) -> None:
        '''
        Parameters
        -------------
        sample : E.g. Bu_JpsiK_ee_eq_DPC
        trigger: Hlt2RD_BuToKpEE_MVA
        '''
        self._sample = sample
        self._trigger= trigger
    # ----------------------
    def get_path(self) -> Path:
        '''
        Returns
        ------------
        Path to JSON file with specification
        '''
        path = Path('/tmp/file.json')
        path.touch(exist_ok=True)

        return path
# --------------------------
