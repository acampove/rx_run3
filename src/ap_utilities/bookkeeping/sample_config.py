'''
Module containing SampleConfig class
'''

from omegaconf             import DictConfig
from dmu.generic           import utilities as gut
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ap_utilities:sample_config')
# ----------------------
class SampleConfig:
    '''
    Class meant to provide configuration object needed by BKKChecker
    '''
    # ----------------------
    def __init__(self, settings : str, samples : str):
        '''
        Parameters
        -------------
        settings: String describing the name of the file with the simulation configuration, e.g. 2024
        samples : String describinb the name of the file with the event types, e.g. by_priority
        '''

        self._cfg_set = gut.load_conf(package='ap_utilities_data', fpath=f'samples/{settings}.yaml')
        self._cfg_sam = gut.load_conf(package='ap_utilities_data', fpath=f'analyses/{samples}.yaml')

    # ----------------------
    def get_config(self, categories : list[str]) -> DictConfig:
        '''
        Parameters
        -------------
        categories: Names of categories from which event types should be taken

        Returns
        -------------
        Configuration with both samples and settings
        '''
        all_event_types = {}
        for name, event_types in self._cfg_sam.items():
            if name not in categories:
                continue

            all_event_types.update(event_types)

        for section in self._cfg_set.values():
            section['evt_type'] = all_event_types

        return self._cfg_set
# ----------------------
