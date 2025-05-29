'''
Module use to hold RDFGetter12 class
'''
import os
import glob

from ROOT                  import RDataFrame
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
        self._sample = self._get_sample(sample)
        self._trigger= self._get_trigger(trigger)
        self._dset   = dset

        self._ntp_dir= 'no_bdt_q2_mass'
        self._version= 'v10.21p3'
    # --------------------------
    def _get_sample(self, sample : str) -> str:
        if sample in ['Bu_Kee_eq_btosllball05_DPC', 'Bu_Kmumu_eq_btosllball05_DPC']:
            return 'sign'

        if sample in ['Bu_JpsiK_ee_eq_DPC', 'Bu_JpsiK_mm_eq_DPC']:
            return 'ctrl'

        raise NotImplementedError(f'Invalid sample: {sample}')
    # --------------------------
    def _get_trigger(self, trigger : str) -> str:
        if trigger == 'Hlt2RD_BuToKpEE_MVA':
            return 'ETOS'

        if trigger == 'Hlt2RD_BuToKpMuMu_MVA':
            return 'MTOS'

        raise NotImplementedError(f'Invalid trigger: {trigger}')
    # --------------------------
    def _add_columns(self, rdf : RDataFrame) -> RDataFrame:
        q2_track_def = (
                        'ROOT::Math::PxPyPzMVector l1(L1_TRACK_PX, L1_TRACK_PY, L1_TRACK_PZ, 0.511);'
                        'ROOT::Math::PxPyPzMVector l2(L2_TRACK_PX, L2_TRACK_PY, L2_TRACK_PZ, 0.511);'
                        'auto ll = l1 + l2;'
                        'return ll.M2();'
                        )

        q2_true_def  = (
                        'ROOT::Math::PxPyPzEVector l1(L1_TRUEP_X, L1_TRUEP_Y, L1_TRUEP_Z, L1_TRUEP_E);'
                        'ROOT::Math::PxPyPzEVector l2(L2_TRUEP_X, L2_TRUEP_Y, L2_TRUEP_Z, L2_TRUEP_E);'
                        'auto ll = l1 + l2;'
                        'return ll.M2();'
                        )

        rdf = rdf.Define('nbrem'     , 'L1_BremMultiplicity + L2_BremMultiplicity')
        rdf = rdf.Define('B_Mass_smr',             'B_M')
        rdf = rdf.Define('q2_smr'    , 'Jpsi_M * Jpsi_M')
        rdf = rdf.Define('q2_track'  ,      q2_track_def)
        rdf = rdf.Define('q2_true'   ,      q2_true_def )
        rdf = rdf.Define('q2_dtf'    ,      q2_true_def ) # Placeholder, DTF branches were not saved in Run12 ntuples.

        return rdf
    # --------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe with dataset
        '''

        cas_dir = os.environ['CASDIR']
        ntp_wc  = (
                f'{cas_dir}/tools/apply_selection/'
                f'{self._ntp_dir}/{self._sample}/'
                f'{self._version}/{self._dset}_{self._trigger}/*.root'
                )

        l_path = glob.glob(ntp_wc)
        npath  = len(l_path)
        if npath == 0:
            raise ValueError(f'No file found in: {ntp_wc}')

        rdf = RDataFrame(self._trigger, l_path)
        rdf = self._add_columns(rdf=rdf)

        return rdf
# --------------------------
