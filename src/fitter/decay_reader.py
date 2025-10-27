'''
This module contains the class DecayReader and its
derived classes KLLDecayReader and KPiLLDecayReader
'''

from fitter.pchain         import PChain
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:decay_reader')
#---------------------------
class DecayReader:
    '''
    Class used to attach decay weights to dataframe of inclusive decays
    '''
    #---------------------------
    def __init__(self):
        '''
        Takes particle chain instances
        '''
        self._Jp_id = 443
        self._Ps_id = 100443
        self._Pi_id = 211
        self._Ph_id = 333
        self._Et_id = 221
        self._Ks_id = 313
        self._Kp_id = 321
        self._KS_id = 310

        self._Bd_id = 511
        self._Bu_id = 521
        self._Bs_id = 531

        #From https://gitlab.cern.ch/LHCb-RD/ewp-rkstz/-/blob/master/analysis/kernel/inc/ConstDef.hpp
        self._Bc         = 541
        self._Lb         = 5122
        self._L0         = 3122
        self._Kst_c      = 323
        self._K_1_1270_z = 10313
        self._K_1_1270_c = 10323
        self._K_2_1430_c = 325
        self._K_2_1430_z = 315
        self._K_1_1400_z = 20313
        self._K_1_1400_c = 20323
        self._K0         = 311
        self._KLong      = 130
        self._Pi0        = 111
        self._M          = 13
        self._E          = 11
        self._Tau        = 15
        self._P          = 2212
        self._N          = 2112
        self._Eta_prime  = 331
        self._Rho0       = 113
        self._Rho_c      = 213
        self._Omega      = 223
        self._D0         = 421
        self._Dp         = 411
        self._Ds         = 431
        self._Bst0       = 513
        self._Bst_plus   = 523
        self._Photon     = 22

        self._weight  : float        = 1.0
        self._l_chain : list[PChain] = []
        self._l_bid   : list[int]    = [self._Bd_id, self._Bu_id, self._Bs_id]
    #---------------------------
    def _get_jpsi_wgt(self, l1 : PChain, l2 : PChain) -> float:
        '''
        Parameters
        ----------------
        l1/2_ch: Particle chain associated to L1/2 lepton

        Returns
        ----------------
        Weight associated to branching ratio corrections for leptons decay chain
        '''
        weight = 1.0
        for bid in  self._l_bid:
            flg_1 = l1.match_upstream( self._Ps_id, bid)
            flg_2 = l2.match_upstream( self._Ps_id, bid)

            if not flg_1 and not flg_2:
                continue

            l1_from_jp = l1.match_mother(self._Jp_id)
            l2_from_jp = l2.match_mother(self._Jp_id)
            if   l1_from_jp or l2_from_jp:
                #weight = 0.6254 / ( 1-0.1741) #0.75
                weight = 0.958

            l1_from_ps = l1.match_mother(self._Ps_id)
            l2_from_ps = l2.match_mother(self._Ps_id)
            if l1_from_ps or l2_from_ps:
                #weight = 1.3200 / 0.1741 #7.58
                weight = 0.771

        return weight
    #---------------------------
    def _get_psi2_wgt(self, l1 : PChain, l2 : PChain) -> float:
        '''
        Parameters
        ----------------
        l1/2_ch: Particle chain associated to L1/2 lepton

        Returns
        ----------------
        Weight associated to branching ratio corrections for leptons decay chain
        when they come directly from a Psi2S
        '''
        weight= 1
        flg_1 = l1.match_upstream(self._Ps_id, self._Bd_id)
        flg_2 = l2.match_upstream(self._Ps_id, self._Bd_id)
        if flg_1 or flg_2:
            weight = 1.17

        flg_1 = l1.match_upstream(self._Ps_id, self._Bu_id)
        flg_2 = l2.match_upstream(self._Ps_id, self._Bu_id)
        if flg_1 or flg_2:
            weight = 1.35

        return weight
    #---------------------------
    def _either_track_has(self, pid : int) -> bool:
        '''
        Parameters
        --------------
        pid: Particle ID from PDG

        Returns
        --------------
        True if particle exists in any of the chains registered
        '''
        if not self._l_chain:
            raise ValueError('No chains were registered')

        for chain in self._l_chain:
            if chain.has_in_chain(pid):
                return True

        return False
    #---------------------------
    def _get_psi2_over_jpsi(self) -> float:
        '''
        Returns correction factor for branching fractions
        associated to psi2S chain
        '''
        flg_ps = self._either_track_has(self._Ps_id)
        if not flg_ps:
            return 1.0

        flg_bu = self._either_track_has(self._Bu_id)
        flg_bd = self._either_track_has(self._Bd_id)
        flg_bs = self._either_track_has(self._Bs_id)

        if flg_bs:
            return ( 5.40E-4/ 1.0800E-3 ) / ( 0.0748/ 0.1077) #0.72

        if flg_bu:
            return ( 6.19E-4/ 1.0006E-3 ) / ( 0.0729/ 0.1595) #1.35

        if flg_bd:
            return ( 5.90E-4/ 1.2700E-3 ) / (0.07610/ 0.1850) #1.13

        return 1.0
#---------------------------
class KLLDecayReader(DecayReader):
    '''
    Class used to attach decay weights for KLL signal candidates
    '''
    def __init__(self, l1 : PChain, l2 : PChain, kp : PChain):
        super().__init__()
        self._l_chain = [l1, l2, kp]

        self._l1 = l1
        self._l2 = l2
        self._kp = kp
    #---------------------------
    def _get_kp_wgt(self) -> float:
        '''
        Returns correction weights for branching fraction associated
        to kaon chain
        '''
        weight = 1.0
        if self._kp.match_decay( [self._Pi_id, self._KS_id             ]):
            weight *= 0.5

        if self._kp.match_decay( [self._Kp_id, self._Ph_id, self._Bd_id]):
            weight *= 0.5/0.9974

        if self._kp.match_decay( [self._Kp_id, self._Ph_id, self._Bu_id]):
            weight *= 0.5/0.7597

        if self._kp.match_decay( [self._Pi_id, self._Et_id] ):
            weight *= 0.28/0.4

        if self._kp.match_decay( [self._Kp_id, self._Ks_id] ):
            weight *= 0.66 / 0.7993

        if self._kp.match_decay( [self._Kp_id, self._Kst_c] ):
            weight *= 0.33 / 0.4993

        if self._kp.match_decay( [self._Kp_id, self._K_2_1430_c]):
            weight *= 0.1670/0.2485

        return weight
    #---------------------------
    def get_weight(self) -> float:
        '''
        Returns:

        wt (float): Weight for candidate
        '''
        w1 = self._get_jpsi_wgt(l1=self._l1, l2=self._l2)
        w2 = self._get_psi2_wgt(l1=self._l1, l2=self._l2)
        w3 = self._get_psi2_over_jpsi()
        w4 = self._get_kp_wgt()

        log.verbose(f'Jpsi wgt: {w1:.3f}')
        log.verbose(f'psi2 wgt: {w1:.3f}')
        log.verbose(f'Psi2 over Jpsi wgt: {w1:.3f}')
        log.verbose(f'Kplus wgt: {w1:.3f}')

        wt = w1 * w2 * w3 * w4

        return wt
#---------------------------
