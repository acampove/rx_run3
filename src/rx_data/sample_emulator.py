'''
Module holding SampleEmulator class
'''

from ROOT                  import RDF # type: ignore

from dmu.generic.utilities import load_conf
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:sample_emulator')
# ----------------------
class SampleEmulator:
    '''
    Class meant to:

    - Open config file with emulation settings
    - Provide emulating sample name 
    - Postprocess original sample to make it suitable for emulation
    '''
    # ----------------------
    def __init__(self, sample : str) -> None:
        '''
        Parameters
        ---------------
        sample: Name of sample, e.g. Bd_JpsiKst_ee_eq_DPC
        '''
        self._sample = sample
        self._cfg    = load_conf(package='rx_data_data', fpath='emulated_trees/config.yaml')
    # ---------------------
    def get_sample_name(self) -> str:
        '''
        Returns
        -------------
        If meant to be emulated, emulating sample, otherwise
        original sample
        '''

        if self._sample not in self._cfg:
            log.debug(f'Not emulating {self._sample}')
            return self._sample

        new_sample = self._cfg[self._sample].sample
        log.warning(f'Emulating {self._sample} with {new_sample}')

        return new_sample
    # ---------------------
    def post_process(self, rdf : RDF.RNode) -> RDF.RNode:
        '''
        Parameters
        -------------
        rdf: ROOT dataFrame

        Returns
        -------------
        Dataframe after redefinitions, etc
        '''
        if self._sample not in self._cfg:
            return rdf

        for key, val in self._cfg[self._sample].redefine.items():
            rdf = rdf.Redefine(key, val)

        return rdf
# ----------------------
