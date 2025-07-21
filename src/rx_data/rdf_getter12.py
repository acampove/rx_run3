'''
Module use to hold RDFGetter12 class
'''
import os
import glob

from ROOT                  import RDataFrame, RDF
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:rdf_getter12')
# --------------------------
class RDFGetter12:
    '''
    This class is meant to allow access to Run1/2 ntuples
    '''
    # --------------------------
    def __init__(self, sample : str, trigger : str, dset : str):
        '''
        sample : Name of data/MC sample, e.g. Bu_Kee_eq_btosllball05_DPC
        trigger: Hlt2 trigger, e.g. Hlt2RD_BuToKpEE_MVA, meant to be translated as ETOS or MTOS
        dset   : Year, e.g. 2018
        '''
        self._dset   = '*' if dset == 'all' else dset
        self._ntp_dir= 'no_bdt_q2_mass'
        self._version= 'v10.21p3'

        self._l_sig_sample = ['Bu_Kee_eq_btosllball05_DPC', 'Bu_Kmumu_eq_btosllball05_DPC']
        self._l_ctr_sample = ['Bu_JpsiK_ee_eq_DPC'        , 'Bu_JpsiK_mm_eq_DPC']

        self._d_trigger    = {
            'Hlt2RD_BuToKpEE_MVA'  : 'ETOS',
            'Hlt2RD_BuToKpMuMu_MVA': 'MTOS'}

        self._sample = self._get_sample(sample)
        self._trigger= self._d_trigger[trigger]
    # --------------------------
    def _get_sample(self, sample : str) -> str:
        '''
        Parameters
        --------------
        sample: Run 3 sample name, e.g. Bu_Kee_eq_btosllball05_DPC

        Returns
        --------------
        Run1/2 sample name, e.g. sign
        '''
        if sample in self._l_sig_sample:
            return 'sign'

        if sample in self._l_ctr_sample:
            return 'ctrl'

        raise NotImplementedError(f'Invalid sample: {sample}')
    # --------------------------
    def get_rdf(self) -> RDF.RNode:
        '''
        Returns ROOT dataframe with dataset
        '''

        cas_dir = os.environ['CASDIR']
        ntp_wc  = (
            f'{cas_dir}/tools/apply_selection/'
            f'{self._ntp_dir}/{self._sample}/'
            f'{self._version}/{self._dset}_{self._trigger}/*.root')

        l_path = glob.glob(ntp_wc)
        npath  = len(l_path)
        if npath == 0:
            raise ValueError(f'No file found in: {ntp_wc}')

        rdf = RDataFrame(self._trigger, l_path)

        return rdf
# --------------------------
