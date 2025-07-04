'''
Module holding MisIDCalculator class
'''

from multiprocessing import Pool

import pandas as pnd

from ROOT                     import RDataFrame
from dmu.logging.log_store    import LogStore
from dmu.generic              import hashing
from dmu.generic              import utilities as gut
from dmu.pdataframe           import utilities as put

from rx_selection             import selection as sel
from rx_data.rdf_getter       import RDFGetter
from rx_misid.splitter        import SampleSplitter
from rx_misid.sample_weighter import SampleWeighter

log=LogStore.add_logger('rx_misid:misid_calculator')
# ----------------------------
class MisIDCalculator:
    '''
    Class meant to apply contamination selection and reweighting for a given sample, e.g.

    Data
    Signal
    Leakage
    '''
    # -----------------------------
    def __init__(self, cfg : dict):
        '''
        cfg: Dictionary with configuration
        '''
        self._cfg = cfg
    # -----------------------------
    def _get_selection(self) -> dict[str,str]:
        '''
        Returns
        ----------------
        Dictionary with full selection, plus control region
        '''
        trigger = self._cfg['input']['trigger']
        q2bin   = self._cfg['input']['q2bin'  ]
        sample  = self._cfg['input']['sample' ]

        d_sel          = sel.selection(trigger=trigger, q2bin=q2bin, process=sample)
        d_sel['pid_l'] = '(1)'

        if 'selection' not in self._cfg['input']:
            return d_sel

        log.warning('Overriding selection')
        d_cut   = self._cfg['input']['selection']
        for name, expr in d_cut.items():
            log.info(f'{name:<name}{expr}')
            d_sel[name] = expr

        return d_sel
    # -----------------------------
    def _get_sample(self, arg : tuple[bool,str]) -> pnd.DataFrame:
        '''
        Method in charge of steering:

        - Reading of data
        - Splitting 
        - Weighting

        This method needs to take one argument to be used with multiprocessing
        '''
        is_bplus, hadron_id = arg

        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']
        project = self._cfg['input']['project']

        log.debug(f'Loading: {sample}/{trigger}/{project}')

        obj     = RDFGetter(sample=sample, trigger=trigger, analysis=project)
        rdf     = obj.get_rdf()
        uid     = obj.get_uid()
        rdf,uid = self._filter_rdf(rdf=rdf, uid=uid)
        rdf.uid = uid

        log.info(f'Splitting samples: Bplus={is_bplus}, Hadron={hadron_id}')
        splitter = SampleSplitter(
                rdf      = rdf,
                sample   = sample,
                is_bplus = is_bplus,
                hadron_id= hadron_id,
                cfg      = self._cfg['splitting'])

        df       = splitter.get_samples()

        log.info('Applying weights')
        weighter = SampleWeighter(df=df, cfg=self._cfg['weights'])
        df       = weighter.get_weighted_data()

        df['hadron'] = hadron_id
        df['bmeson'] = 'bplus' if is_bplus else 'bminus'

        df = put.dropna(df, max_frac=0.04)

        return df
    # -----------------------------
    def _filter_rdf(
            self,
            rdf : RDataFrame,
            uid : str) -> tuple[RDataFrame,str]:
        '''
        Take ROOT dataframe and its UniqueIDentifier

        Filter by:

        - Select range of entries (optional)
        - Apply analysis selection

        Returns
        -----------------
        Filtered dataframe and updated UniqueIDentifier
        '''
        entry_range = self._cfg['input'].get('range')
        if entry_range is not None:
            log.warning(f'Limitting dataframe to {entry_range}')
            min_entry, max_entry = entry_range
            rdf = rdf.Range(min_entry, max_entry)

        d_sel   = self._get_selection()
        log.info('Applying selection')
        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<30}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        uid = hashing.hash_object(obj=[d_sel, uid, entry_range])

        return rdf, uid
    # -----------------------------
    def get_misid(self) -> pnd.DataFrame:
    @gut.timeit
        '''
        Returns pandas dataframe with weighted entries with, extra columns
        hadron : kaon or pion
        bmeson : bplus or bminus

        For a given kind of inputs, e.g (Data, signal, leakage)
        '''
        l_arg = [ (x, y) for x in [True,False] for y in ['kaon', 'pion'] ]
        nproc = len(l_arg)

        with Pool(processes=nproc) as pool:
            l_df = pool.map(self._get_sample, l_arg)

        df = pnd.concat(l_df)

        return df
# -----------------------------
