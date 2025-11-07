'''
Module holding SampleEmulator class
'''

from ROOT                  import RDF # type: ignore
from omegaconf             import DictConfig

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
        sample: Name of sample to emulate, e.g. Bs_JpsiKst_ee_eq_DPC
        '''
        self._sample = sample
        self._cfg    = load_conf(package='rx_data_data', fpath='emulated_trees/config.yaml').get(sample)
    # ---------------------
    def get_sample_name(self) -> str:
        '''
        Returns
        -------------
        If meant to be emulated, emulating sample, otherwise
        original sample
        '''

        if self._cfg is None:
            log.debug(f'Not emulating {self._sample}')
            return self._sample

        new_sample = self._cfg.sample
        log.warning(f'Emulating {self._sample} with {new_sample}')

        return new_sample
    # ----------------------
    def _run_redefine(
        self, 
        rdf         : RDF.RNode, 
        definitions : DictConfig) -> RDF.RNode:
        '''
        Parameters
        -------------
        rdf : DataFrame before redefinitions
        definitions: Dictionary between variable and new definition

        Returns
        -------------
        Dictionary after definitiions
        '''
        for var, expr in definitions.items():
            assert isinstance(var, str)

            log.debug(f'{var:<30}{"--->"}{expr:<}')
            rdf = rdf.Redefine(var, expr)

        return rdf
    # ----------------------
    def extend_redefinitions(self, redefinitions : dict[str,str]) -> None:
        '''
        Add extra entries to redefinition section of config

        Parameters
        -------------
        redefinitions: Dictionary mapping column name to new definition

        Raises 
        -------------
        ValueError: If a column re-definition is already present in config
        '''
        if not redefinitions:
            log.info('Not using extra redefinitions')
            return

        fail = False
        log.info('Extending columns redefinitions')
        for name, expression in redefinitions.items():
            if name in self._cfg.redefine:
                log.error(name)
                fail = True
                continue

            log.debug(name)
            self._cfg.redefine[name] = expression

        if fail:
            raise ValueError('Cannot redefine, some columns are already found in config')
    # ----------------------
    def post_process(self, rdf : RDF.RNode) -> RDF.RNode:
        '''
        Parameters
        -------------
        rdf: ROOT dataFrame

        Returns
        -------------
        Dataframe after redefinitions, etc
        '''
        if self._cfg is None: 
            log.debug(f'Not emulating {self._sample}, missing in config')
            return rdf

        log.info(f'Emulating {self._sample}')

        if 'redefine' in self._cfg:
            rdf = self._run_redefine(rdf=rdf, definitions=self._cfg.redefine)

        return rdf
# ----------------------
