'''
This module is meant to hold SamplesPrinter
'''

import os
import tqdm
import math
import numpy
import pandas as pnd 

from colorama               import Fore
from pathlib                import Path
from ROOT                   import RDF # type: ignore
from rx_common.types        import Project
from rx_data                import utilities as dut
from rx_data.rdf_getter     import RDFGetter
from dmu.generic            import version_management as vm
from dmu.logging.log_store  import LogStore
from dmu.pdataframe         import utilities as put

log=LogStore.add_logger('rx_data:samples')
# ----------------------
class SamplesPrinter:
    '''
    This class is meant to print data and MC samples by block
    '''
    # ----------------------
    def __init__(self, project : Project) -> None:
        '''
        Parameters
        -------------
        project: Project, e.g. rk, rkst, rk_no_refit
        '''
        self._project = project
        self._skipped_samples = ['Bu_D0pi_Kenu_eq_DPC_TC']

        LogStore.set_level('rx_data:rdf_getter', 40)
    # ----------------------
    def _get_input_samples(self) -> list[tuple[str,str]]:
        '''
        Returns
        -------------
        List of tuples with sample name and trigger name
        '''
        ana_dir   = Path(os.environ['ANADIR'])
        main_path = ana_dir / f'Data/{self._project}/main'
        vers_path = vm.get_last_version(dir_path = main_path, version_only=False)
        paths     = vers_path.glob('*.root')
        if not paths:
            raise ValueError(f'No ROOT files found in: {vers_path}')

        s_sample_trigger = { dut.info_from_path(path, sample_lowercase=False) for path in paths }
        nsamples         = len(s_sample_trigger)

        log.info(f'Found {nsamples} samples')

        return list(s_sample_trigger)
    # ----------------------
    def _get_rdf(self) -> dict[str, RDF.RNode]:
        '''
        Returns
        -------------
        Dictionary mapping sample name to corresponding dataframe built from only main trees
        '''
        log.info('Collecting dataframes')
        input_samples : list[tuple[str,str]] = self._get_input_samples()
        d_rdf : dict[str, RDF.RNode]         = dict()

        with RDFGetter.only_friends(s_friend=set()),\
             RDFGetter.project(name = self._project):
            for sample, trigger in tqdm.tqdm(input_samples, ascii=' -'):
                if sample in self._skipped_samples:
                    continue

                gtr = RDFGetter(sample =sample, trigger=trigger)
                try:
                    d_rdf[sample] = gtr.get_rdf(per_file=False)
                except ValueError:
                    log.warning(f'Cannot read {sample}, skipping')

        return d_rdf
    # ----------------------
    def _get_block_stats(self, rdf : RDF.RNode) -> dict[str,str]:
        '''
        Parameters
        -------------
        rdf : ROOT dataframe with main sample 

        Returns
        -------------
        dictionary where:

        Key  : Block number
        Value: Percentage of events in block
        '''
        arr_block      = rdf.AsNumpy(['block'])['block']
        values, counts = numpy.unique(arr_block, return_counts=True)
        total          = sum(counts)
        fractions      = [ math.floor(100 * count / total) for count in counts ]
        values         = [ f'Block {value:.0f}'            for value in values ]
        d_stats        = dict(zip(values, fractions))
        d_stats_sorted = { key : d_stats[key] for key in sorted(d_stats) }

        return d_stats_sorted
    # ----------------------
    def print_by_block(self) -> None:
        '''
        Prints a table with block column and samples as the rows
        '''
        d_rdf = self._get_rdf()

        l_d_block_stats : list[dict[str,str]] = []

        log.info('Collecting statistics')
        for rdf in tqdm.tqdm(d_rdf.values(), ascii=' -'):
            d_block_stats : dict[str,str] = self._get_block_stats(rdf=rdf)
            l_d_block_stats.append(d_block_stats)

        df = pnd.DataFrame(l_d_block_stats, index=list(d_rdf))
        df = df.sort_index()
        df = df[sorted(df.columns)]
        df = df.fillna(value=0)

        colors = {0 : Fore.LIGHTRED_EX}
        df = df.apply(put.colorize_row, args=(colors,), axis=1)

        print(df.to_markdown())
# ----------------------
