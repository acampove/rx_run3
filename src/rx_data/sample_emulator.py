'''
Module holding SampleEmulator class
'''

from ROOT                  import RDF # type: ignore
from omegaconf             import DictConfig, ListConfig

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
    def _run_swaps(
        self, 
        rdf   : RDF.RNode, 
        swaps : ListConfig) -> RDF.RNode:
        '''
        Parameters
        -------------
        rdf: DataFrame before swaps
        swaps: List with two elements, representing heads of branches to swap, e.g. H1, H2

        Returns
        -------------
        Dataframe with branches values swapped
        '''
        if len(swaps) != 2:
            raise ValueError(f'Expected two particles, found: {swaps}')

        log.info(f'Swapping: {swaps}')
        [part_1, part_2] = swaps

        l_part_1 = sorted([ name.c_str() for name in rdf.GetColumnNames() if name.c_str().startswith(part_1) ])
        l_part_2 = sorted([ name.c_str() for name in rdf.GetColumnNames() if name.c_str().startswith(part_2) ])

        assert len(l_part_1) == len(l_part_2)

        for part_1, part_2 in zip(l_part_1, l_part_2):
            rdf = rdf.Define(f'tmp_{part_1}', part_1)
            rdf = rdf.Define(f'tmp_{part_2}', part_2)

            rdf = rdf.Redefine(part_2, f'tmp_{part_1}')
            rdf = rdf.Redefine(part_1, f'tmp_{part_2}')

        return rdf
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
        cfg = self._cfg.get(self._sample)

        if not cfg: 
            log.debug(f'Not emulating {self._sample}, missing in config')
            return rdf

        log.info(f'Emulating {self._sample}')

        if 'redefine' in cfg:
            rdf = self._run_redefine(rdf=rdf, definitions=cfg.redefine)

        if 'swap' in cfg:
            rdf = self._run_swaps(rdf=rdf, swaps=cfg.swap)

        return rdf
# ----------------------
