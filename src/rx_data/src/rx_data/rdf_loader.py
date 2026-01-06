'''
Module holding RDFLoader class
'''

from pathlib import Path
from ROOT    import RDF  # type: ignore
from dmu     import LogStore

log=LogStore.add_logger('rx_data:rdf_loader')
# ----------------------------------
class RDFLoader:
    '''
    Class meant to hold functionality to provide ROOT dataframes.
    It is supposed to handle:

    - Network delays
    - Timeouts
    '''
    # ----------------------
    @staticmethod
    def from_conf(
        ntries : int,
        wait   : int,
        path   : Path) -> RDF.RNode:
        '''
        Parameters
        -------------
        ntries: Number of times to retry loading of data before raising exception
        wait  : Number of seconds to wait between failed tries
        path: Path to JSON file holding configuration needed to build dataframe

        Returns
        -------------
        ROOT dataframe
        '''
        rdf = RDF.Experimental.FromSpec(str(path))

        return rdf
# ----------------------------------
