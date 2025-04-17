'''
Module holding MisIDCalculator class
'''

from zfit.core.data         import Data      as zdata
from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter

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
    def _get_sample(self, kind : str, is_bplus : bool) -> zdata:
        sample  = self._cfg['input']['samples'][kind]
        trigger = self._cfg['input']['trigger']
        d_cut   = self._cfg['input']['selection']

        obj     = RDFGetter(sample=sample, trigger=trigger)
        rdf     = obj.get_rdf()

        d_sel   = sel.selection(project='RK', analysis='EE', q2bin='jpsi', process='DATA')
        d_sel.update(d_cut)

        for cut_name, cut_expr in d_sel.items():
            rdf = rdf.Filter(cut_expr, cut_name)

        splitter = SampleSplitter(rdf=rdf, is_bplus=is_bplus)
        d_sample = splitter.get_samples()

        weighter = SampleWeighter(samples=d_sample, cfg=cfg['maps'])
        data     = weighter.get_weighted_data()

        return data
    # -----------------------------
    def _get_dataset(self, is_bplus : bool) -> zdata:
        '''
        Returns zfit dataset with weighted events
        representing contamination

        is_bplus : Sign of the B meson 
        '''
        data_msid = self._get_sample(kind=  'data', is_bplus = is_bplus)
        data_sign = self._get_sample(kind='signal', is_bplus = is_bplus)
        data_leak = self._get_sample(kind=  'leak', is_bplus = is_bplus)

        return data_msid - data_sign - data_leak
    # -----------------------------
    def get_misid(self) -> zdata:
        '''
        Returns zfit dataset with weighted events
        '''

        bp_misid=self._get_dataset(is_bplus= True)
        bm_misid=self._get_dataset(is_bplus=False)

        return bp_misid + bm_misid
# -----------------------------
