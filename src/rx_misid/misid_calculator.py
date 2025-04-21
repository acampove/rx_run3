'''
Module holding MisIDCalculator class
'''

from dmu.stats.wdata        import Wdata
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
    def _get_sample(self, is_bplus : bool, hadron_id : str) -> Wdata:
        sample  = self._cfg['input']['sample']
        trigger = self._cfg['input']['trigger']
        d_cut   = self._cfg['input']['selection']

        obj     = RDFGetter(sample=sample, trigger=trigger)
        rdf     = obj.get_rdf()

        max_entries = self._cfg['input'].get('max_entries')
        if max_entries is not None:
            log.warning(f'Limitting dataframe to {max_entries}')
            rdf = rdf.Range(max_entries)

        d_sel   = self._get_selection(cuts=d_cut)
        log.info('Applying selection')
        for cut_name, cut_expr in d_sel.items():
            log.debug(f'{cut_name:<30}{cut_expr}')
            rdf = rdf.Filter(cut_expr, cut_name)

        log.info('Splitting samples')
        splitter = SampleSplitter(rdf=rdf, is_bplus=is_bplus, hadron_id=hadron_id, cfg=self._cfg['splitting'])
        d_sample = splitter.get_samples()

        log.info('Applying weights')
        weighter = SampleWeighter(samples=d_sample, cfg=self._cfg['maps'])
        d_data   = weighter.get_weighted_data()

        total_data = None
        for kind, data in d_data.items():
            if data.size == 0:
                log.warning('Found empty dataset for:')
                log.info(f'{"Kind  ":<20}{kind:<20}')
                log.info(f'{"B+    ":<20}{is_bplus:<20}')
                log.info(f'{"Hadron":<20}{hadron_id:<20}')
            if total_data is None:
                total_data = data
                continue

            total_data = total_data + data

        return total_data
    # -----------------------------
    def get_misid(self) -> Wdata:
        '''
        Returns zfit dataset with weighted events after adding
        - Kaon/Pion like
        - Bplus and Bminus

        For a given kind of inputs, e.g (Data, signal, leakage)
        '''

        bp_misid_kaon=self._get_sample(is_bplus= True, hadron_id='kaon')
        bm_misid_kaon=self._get_sample(is_bplus=False, hadron_id='kaon')

        bp_misid_pion=self._get_sample(is_bplus= True, hadron_id='pion')
        bm_misid_pion=self._get_sample(is_bplus=False, hadron_id='pion')

        kaon_misid = bp_misid_kaon + bm_misid_kaon
        pion_misid = bp_misid_pion + bm_misid_pion

        return kaon_misid + pion_misid
# -----------------------------
