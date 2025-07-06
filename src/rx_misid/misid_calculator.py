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
from rx_misid.sample_splitter import SampleSplitter
from rx_misid.sample_weighter import SampleWeighter

log=LogStore.add_logger('rx_misid:misid_calculator')
# ----------------------------
class MisIDCalculator:
    '''
    Class meant to provide a dataframe for a given sample among:

    Data
    Signal
    Leakage

    In either the signal or the control region
    '''
    # -----------------------------
    def __init__(
            self,
            cfg    : dict,
            is_sig : bool):
        '''
        cfg   : Dictionary with configuration
        is_sig: If true/false, provides dataframes with weights to transfer sample to signal/contrl region
        '''
        self._cfg   =    cfg
        self._is_sig= is_sig
        self._b_id  = 521

        self._l_track = self._get_tracks()
    # -----------------------------
    def _get_tracks(self) -> list[str]:
        '''
        Given the data sample that this instance processes
        this method will decide if the electron tracks should
        be treated as kaons, pions, electrons

        This is needed to switch between PID calibration maps
        '''

        sample = self._cfg['input']['sample']
        if sample.startswith('DATA_'):
            return ['kaon', 'pion']

        if sample == 'Bu_piplpimnKpl_eq_sqDalitz_DPC':
            return ['pion']

        if sample == 'Bu_KplKplKmn_eq_sqDalitz_DPC':
            return ['kaon']

        if sample in ['Bu_Kee_eq_btosllball05_DPC', 'Bu_JpsiK_ee_eq_DPC']:
            return ['electron']

        raise ValueError(f'Unrecognized sample: {sample}')
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
            log.info(f'{name:<20}{expr}')
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
        rdf,uid = self._filter_rdf(rdf=rdf, uid=uid, is_bplus=is_bplus)
        rdf.uid = uid

        splitter     = SampleSplitter(
            rdf      = rdf,
            sample   = sample,
            is_bplus = is_bplus,
            hadron_id= hadron_id,
            cfg      = self._cfg['splitting'])

        df = splitter.get_samples()

        log.info('Applying weights')
        weighter = SampleWeighter(
                df    = df,
                cfg   = self._cfg['weights'],
                sample= sample,
                is_sig= self._is_sig)

        df = weighter.get_weighted_data()


        # TODO: Use replace_nan to replace nans with 1s
        # This should drop up to 6% of the dataset
        # Due to NaNs in the PID maps.
        df = put.dropna(df, max_frac=0.06)

        return df
    # -----------------------------
    def _filter_rdf(
            self,
            rdf      : RDataFrame,
            is_bplus : bool,
            uid      : str) -> tuple[RDataFrame,str]:
        '''
        Parameters
        ----------------
        rdf     : ROOT dataframe
        uid     : Dataframe unique identifier
        is_bplus: To get only B+ candidates

        Filter by:

        - Select range of entries (optional)
        - Apply analysis selection

        Returns
        -----------------
        Filtered dataframe and updated UniqueIDentifier
        '''
        entry_range = self._cfg['input'].get('range')
        if entry_range is not None:
            log.warning(f'Limiting dataframe to {entry_range}')
            min_entry, max_entry = entry_range
            rdf = rdf.Range(min_entry, max_entry)

        bid            = self._b_id if is_bplus else -self._b_id
        d_sel          = self._get_selection()
        d_sel['block'] = 'block > 0'
        d_sel['B_ID' ] =f'B_ID=={bid}'

        log.info('Applying selection')
        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<30}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        uid = hashing.hash_object(obj=[d_sel, uid, entry_range])

        return rdf, uid
    # -----------------------------
    @gut.timeit
    def get_misid(self, multi_proc : bool = True) -> pnd.DataFrame:
        '''
        Parameters
        -------------------
        multi_proc: If true will process four (2 charges x 2 hadron IDs) in parallel, default False

        Returns
        -------------------
        pandas dataframe with weighted entries with, extra columns
        hadron : kaon or pion
        bmeson : bplus or bminus

        For a given kind of inputs, e.g (Data, signal, leakage, hadronic misID)
        '''
        l_arg = [ (x, y) for x in [True,False] for y in self._l_track ]

        if multi_proc:
            nproc = len(l_arg)
            log.warning(f'Using multiprocessing with {nproc} processes')
            with Pool(processes=nproc) as pool:
                l_df = pool.map(self._get_sample, l_arg)
        else:
            l_df = [ self._get_sample(arg) for arg in l_arg ]

        log.debug('Merging dataframes')
        df = pnd.concat(l_df)

        return df
# -----------------------------
