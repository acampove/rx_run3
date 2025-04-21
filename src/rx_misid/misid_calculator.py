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
    Class meant to calculate MisID contamination
    '''
    # -----------------------------
    def __init__(self, cfg : dict):
        '''
        cfg: Dictionary with configuration
        '''
        self._cfg = cfg
    # -----------------------------
    def _get_sample(self, kind : str, is_bplus : bool, hadron_id : str) -> Wdata:
        sample  = self._cfg['input']['samples'][kind]
        trigger = self._cfg['input']['trigger']
        d_cut   = self._cfg['input']['selection']

        obj     = RDFGetter(sample=sample, trigger=trigger)
        rdf     = obj.get_rdf()

        d_sel   = sel.selection(project='RK', analysis='EE', q2bin='jpsi', process='DATA')
        d_sel.update(d_cut)

        for cut_name, cut_expr in d_sel.items():
            rdf = rdf.Filter(cut_expr, cut_name)

        splitter = SampleSplitter(rdf=rdf, is_bplus=is_bplus, hadron_id=hadron_id, cfg=self._cfg['splitting'])
        d_sample = splitter.get_samples()

        weighter = SampleWeighter(samples=d_sample, cfg=self._cfg['maps'])
        d_data   = weighter.get_weighted_data()
        data     = sum(d_data.values())

        return data
    # -----------------------------
    def _get_dataset(self, is_bplus : bool, hadron_id : str) -> Wdata:
        '''
        Returns zfit dataset with weighted events
        representing contamination

        is_bplus : Sign of the B meson 
        '''
        data_msid = self._get_sample(kind=  'data', is_bplus = is_bplus, hadron_id=hadron_id)
        data_sign = self._get_sample(kind='signal', is_bplus = is_bplus, hadron_id=hadron_id)
        data_leak = self._get_sample(kind=  'leak', is_bplus = is_bplus, hadron_id=hadron_id)

        return data_msid - data_sign - data_leak
    # -----------------------------
    def get_misid(self) -> Wdata:
        '''
        Returns zfit dataset with weighted events
        '''

        bp_misid_kaon=self._get_dataset(is_bplus= True, hadron_id='kaon')
        bm_misid_kaon=self._get_dataset(is_bplus=False, hadron_id='kaon')

        bp_misid_pion=self._get_dataset(is_bplus= True, hadron_id='pion')
        bm_misid_pion=self._get_dataset(is_bplus=False, hadron_id='pion')

        kaon_misid = bp_misid_kaon + bm_misid_kaon
        pion_misid = bp_misid_pion + bm_misid_pion

        return kaon_misid + pion_misid
# -----------------------------
