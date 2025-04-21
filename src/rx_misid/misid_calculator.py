'''
Module holding MisIDCalculator class
'''

import pandas as pnd

from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter
from rx_misid.splitter      import SampleSplitter
from rx_misid.weighter      import SampleWeighter

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
    def _get_selection(self, cuts : dict[str,str]) -> dict[str,str]:
        q2bin     = self._cfg['input']['q2bin']
        sample    = self._cfg['input']['sample']
        d_sel_org = sel.selection(project='RK', analysis='EE', q2bin=q2bin, process=sample)
        d_sel = {}

        for cut_name, cut_expr in d_sel_org.items():
            if 'pid' in cut_name:
                log.info(f'Skipping: {cut_name}={cut_expr}')
                continue

            d_sel[cut_name] = cut_expr

        d_sel.update(cuts)

        return d_sel
    # -----------------------------
    def _get_sample(self, is_bplus : bool, hadron_id : str) -> pnd.DataFrame:
        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']
        d_cut   = self._cfg['input']['selection']

        obj     = RDFGetter(sample=sample, trigger=trigger)
        rdf     = obj.get_rdf()

        entry_range = self._cfg['input'].get('range')
        if entry_range is not None:
            log.warning(f'Limitting dataframe to {entry_range}')
            min_entry, max_entry = entry_range
            rdf = rdf.Range(min_entry, max_entry)

        d_sel   = self._get_selection(cuts=d_cut)
        log.info('Applying selection')
        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<30}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        log.info('Splitting samples')
        splitter = SampleSplitter(rdf=rdf, is_bplus=is_bplus, hadron_id=hadron_id, cfg=self._cfg['splitting'])
        df       = splitter.get_samples()

        log.info('Applying weights')
        weighter = SampleWeighter(df=df, cfg=self._cfg['weights'])
        df       = weighter.get_weighted_data()

        df['hadron'] = hadron_id
        df['bmeson'] = 'bplus' if is_bplus else 'bminus'

        return df
    # -----------------------------
    def get_misid(self) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with weighted entries with, extra columns
        hadron : kaon or pion
        bmeson : bplus or bminus

        For a given kind of inputs, e.g (Data, signal, leakage)
        '''

        df_k_bp =self._get_sample(is_bplus= True, hadron_id='kaon')
        df_k_bm =self._get_sample(is_bplus=False, hadron_id='kaon')

        df_pi_bp=self._get_sample(is_bplus= True, hadron_id='pion')
        df_pi_bm=self._get_sample(is_bplus=False, hadron_id='pion')

        l_df = [df_k_bp, df_k_bm, df_pi_bp, df_pi_bm]

        return pnd.concat(l_df)
# -----------------------------
