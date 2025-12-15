'''
This module contains classes derived from Enum
'''
from enum import IntEnum, StrEnum

# ---------------------------------------
class Brem(IntEnum):
    '''
    Enum meant to represent brem category
    '''
    zero = 0 
    one  = 1 
    two  = 2 
# ---------------------------------------
class Component(StrEnum):
    r'''
    This class is meant to hold elements representing fitting components 

    data          : Real data
    jpsi          : E.g. B^+ -> J/psi K^+, B^0 J/psi K^*
    psi2          : E.g. B^+ -> psi(2s) K^+, B^0 psi(2S) K^*
    ccbar         : Charmonium inclusive samples
    cabibbo       : B^+ \to \pi^+ J/\psi
    lbjpsipk      : Lb -> p K J/psi
    bsjpsiphi     : B_s \to J/\psi \phi
    bsjpsikst     : B_s \to J/\psi K^* 
    bdjpsikst_swp : B_d \to J/\psi K^* with K -> pi and pi -> K
    '''
    data          = 'data'
    jpsi          = 'jpsi'
    psi2          = 'psi2'
    ccbar         = 'ccbar'
    cabibbo       = 'cabibbo'
    lbjpsipk      = 'lbjpsipk'
    bsjpsiphi     = 'bsjpsiphi'
    bsjpsikst     = 'bsjpsikst'
    bdjpsikst_swp = 'bdjpsikst_swp'
# ---------------------------------------
class Trigger(StrEnum):
    '''
    Class meant to represent MVA triggers
    '''
    rk_ee_os     = 'Hlt2RD_BuToKpEE_MVA'
    rk_ee_ext    = 'Hlt2RD_BuToKpEE_MVA_ext'
    rk_ee_ss     = 'Hlt2RD_BuToKpEE_SameSign_MVA'
    rk_ee_nopid  = 'Hlt2RD_BuToKpEE_MVA_noPID'
    rk_ee_cal    = 'Hlt2RD_BuToKpEE_MVA_cal'
    rk_ee_misid  = 'Hlt2RD_BuToKpEE_MVA_misid'
    rk_mm_os     = 'Hlt2RD_BuToKpMuMu_MVA'
    rk_mm_nopid  = 'Hlt2RD_BuToKpMuMu_MVA_noPID'
    rk_mm_ss     = 'Hlt2RD_BuToKpMuMu_SameSign_MVA'
    # -----------
    rkst_ee_os   = 'Hlt2RD_B0ToKpPimEE_MVA'
    rkst_ee_ext  = 'Hlt2RD_B0ToKpPimEE_MVA_ext'
    rkst_ee_ss   = 'Hlt2RD_B0ToKpPimEE_SameSign_MVA'
    rkst_ee_nopid= 'Hlt2RD_B0ToKpPimEE_MVA_noPID'
    rkst_ee_cal  = 'Hlt2RD_B0ToKpPimEE_MVA_cal'
    rkst_ee_misid= 'Hlt2RD_B0ToKpPimEE_MVA_misid'
    rkst_mm_os   = 'Hlt2RD_B0ToKpPimMuMu_MVA'
    rkst_mm_nopid= 'Hlt2RD_B0ToKpPimMuMu_MVA_noPID'
    rkst_mm_ss   = 'Hlt2RD_B0ToKpPimMuMu_SameSign_MVA'
    # -----------
    uninitialized= ''

    def __str__(self):
        return self.value
# ---------------------------------------
class Channel(StrEnum):
    '''
    This class repsesents the electron, muon or emu channel types
    '''
    ee = 'ee'
    mm = 'mm'
    em = 'em'

    def __str__(self):
        return self.value
# ---------------------------------------
class Project(StrEnum):
    '''
    This class represents the projects
    '''
    rk            = 'rk'
    rk_no_pid     = 'rk_nopid'
    rk_no_refit   = 'rk_no_refit'
    rk_sim10d     = 'rk_sim10d'
    # ---------------
    rkst          = 'rkst'
    rkst_no_pid   = 'rkst_nopid'
    rkst_no_refit = 'rkst_no_refit'
    rkst_sim10d   = 'rkst_sim10d'

    def __str__(self):
        return self.value
# ---------------------------------------
class Qsq(StrEnum):
    '''
    This class represents different q2 bins
    '''
    low     = 'low'
    central = 'central'
    jpsi    = 'jpsi'
    psi2    = 'psi2'
    high    = 'high'

    def __str__(self):
        return self.value
# ---------------------------------------
class Sample(StrEnum):
    '''
    Class meant to represent MC or data sample
    '''
    data_24     = 'DATA_24*'
    # -----
    bukee       = 'Bu_Kee_eq_btosllball05_DPC'
    bukjpsiee   = 'Bu_JpsiK_ee_eq_DPC'
    bukpsi2ee   = 'Bu_psi2SK_ee_eq_DPC'
    # -----
    bukmm       = 'Bu_Kmumu_eq_btosllball05_DPC'
    bukjpsimm   = 'Bu_JpsiK_mm_eq_DPC'
    bukpsi2mm   = 'Bu_psi2SK_mm_eq_DPC'
    # -----
    bdkstee     = 'Bd_Kstee_eq_btosllball05_DPC'
    bdkstjpsiee = 'Bd_JpsiKst_ee_eq_DPC'
    bdkstpsi2ee = 'Bd_psi2SKst_ee_eq_DPC'
    # -----
    bdkstmm     = 'Bd_Kstmumu_eq_btosllball05_DPC'
    bdkstjpsimm = 'Bd_JpsiKst_mm_eq_DPC'
    bdkstpsi2mm = 'Bd_psi2SKst_mm_eq_DPC'

    # --------------------------------------------
    @classmethod
    def get_mc_samples(cls) -> list['Sample']:
        '''
        Returns
        ---------------
        List of MC samples known to analysis
        '''

        return [ sample for sample in cls if not sample.name.startswith('data') ] 
    # --------------------------------------------
    def __str__(self):
        '''
        Returns
        ----------------
        String representing sample name, e.g. Bu_JpsiK_ee_eq_DPC
        '''
        return self.value
    # --------------------------------------------
    @property
    def latex(self) -> str:
        '''
        Returns
        ----------------
        Latex string for decay associated to sample
        '''
        return self.name
# ---------------------------------------
