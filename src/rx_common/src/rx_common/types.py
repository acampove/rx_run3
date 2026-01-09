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
    Class meant to represent MVA HLT2 triggers
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
    uninitialized= 'uninitialized'

    def __str__(self):
        return self.value
    # -----------
    @property
    def channel(self) -> str:
        '''
        Either 'muon' or 'electron'
        '''
        if 'EE_MVA'            in self.value:
            return 'electron'

        if 'EE_SameSign_MVA'   in self.value:
            return 'electron'

        if 'MuMu_MVA'          in self.value:
            return 'muon'
        
        if 'MuMu_SameSign_MVA' in self.value:
            return 'muon'

        raise ValueError(f'Cannot determine channel for trigger: {self}')
    # -----------
    @property
    def is_ss(self) -> bool:
        '''
        True if it is a same sign trigger
        '''
        return 'SameSign' in self.value
    # -----------
    @property
    def project(self) -> 'Project':
        '''
        Returns
        -----------------
        Project for which this trigger is meant to be used, e.g. rk, rkst etc
        '''
        if self.name.startswith('rk_'):
            return Project.rk

        if self.name.startswith('rkst_'):
            return Project.rkst

        raise ValueError(f'Cannot assign trigger {self} to any project')
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

    Naming constraints:

    - Except for data, all samples meant to be used with the electron/muon channel should end with ee/mm
    '''
    undefined      = 'undefined'
    # -----
    ccbar          = 'ccbar' # Only needed as placeholder for inclusive charmonium mix
    # -----
    data_24        = 'DATA_24*'
    # -----
    bpkpee         = 'Bu_Kee_eq_btosllball05_DPC'
    bpkpjpsiee     = 'Bu_JpsiK_ee_eq_DPC'
    bpkppsi2ee     = 'Bu_psi2SK_ee_eq_DPC'
    # -----
    bpkpmm         = 'Bu_Kmumu_eq_btosllball05_DPC'
    bpkpjpsimm     = 'Bu_JpsiK_mm_eq_DPC'
    bpkppsi2mm     = 'Bu_psi2SK_mm_eq_DPC'
    # -----
    bppipjpsiee    = 'Bu_JpsiPi_ee_eq_DPC'
    bppipjpsimm    = 'Bu_JpsiPi_mm_eq_DPC'
    # -----
    bdkstkpiee     = 'Bd_Kstee_eq_btosllball05_DPC'
    bdkstkpijpsiee = 'Bd_JpsiKst_ee_eq_DPC'
    bdkstkpipsi2ee = 'Bd_psi2SKst_ee_eq_DPC'
    # -----
    bdkstkpimm     = 'Bd_Kstmumu_eq_btosllball05_DPC'
    bdkstkpijpsimm = 'Bd_JpsiKst_mm_eq_DPC'
    bdkstkpipsi2mm = 'Bd_psi2SKst_mm_eq_DPC'
    # -----
    bpk1kpipiee    = 'Bu_K1ee_eq_DPC'
    bpk2kpipiee    = 'Bu_K2stee_Kpipi_eq_mK1430_DPC'
    bpkstkpiee     = 'Bu_Kstee_Kpi0_eq_btosllball05_DPC'
    bsphiee        = 'Bs_phiee_eq_Ball_DPC'
    # -----
    bpjpsixee      = 'Bu_JpsiX_ee_eq_JpsiInAcc'
    bdjpsixee      = 'Bd_JpsiX_ee_eq_JpsiInAcc'
    bsjpsixee      = 'Bs_JpsiX_ee_eq_JpsiInAcc'
    # -----
    bpjpsixmm      = 'Bu_JpsiX_mm_eq_JpsiInAcc'
    bdjpsixmm      = 'Bd_JpsiX_mm_eq_JpsiInAcc'
    bsjpsixmm      = 'Bs_JpsiX_mm_eq_JpsiInAcc'
    # -----
    # Hadronic misid
    # -----
    bpkkk          = 'Bu_KplKplKmn_eq_sqDalitz_DPC'
    bpkpik         = 'Bu_KplpiplKmn_eq_sqDalitz_DPC'
    bpkpipi        = 'Bu_piplpimnKpl_eq_sqDalitz_DPC' 
    # --------------------------------------------
    @property
    def subdecays(self) -> list[str]:
        '''
        Returns list of subdecays
        '''
        match self.name:
            case 'bpkpee':
                return ['bpkp']
            case 'bpkpmm':
                return ['bpkp']
            case 'bdkstkpiee':
                return ['bdks', 'k+kp']
            case 'bdkstkpimm':
                return ['bdks', 'k+kp']
            # ------
            case 'bpkpjpsiee':
                return ['bpjk', 'jpee']
            case 'bpkpjpsimm':
                return ['bpjk', 'jpmm']
            case 'bpkppsi2ee':
                return ['bppsk', 'psee']
            case 'bpkppsi2mm':
                return ['bppsk', 'psmm']
            # ------
            case 'bdkstkpijpsiee':
                return ['bdjkst' , 'jpee', 'kstkpi']
            case 'bdkstkpijpsimm':
                return ['bdjkst' , 'jpmm', 'kstkpi']
            case 'bdkstkpipsi2ee':
                return ['bdpskst', 'psee', 'kstkpi']
            case 'bdkstkpipsi2mm':
                return ['bdpskst', 'psmm', 'kstkpi']
            # ------
            case 'bsphiee':
                return ['bsph', 'phkk']
            case 'bpk1kpipiee':
                return ['bpk1', 'k13h']
            case 'bpk2kpipiee':
                return ['bpk2', 'k23h']
            case 'bpkstkpiee':
                return ['bpks', 'kokp']
            case _:
                raise NotImplementedError(f'Cannot find subdecays for: {self.name}') 
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
    # --------------------------------------------
    @property
    def channel(self) -> Channel:
        '''
        Returns
        ----------------
        Channel to which current sample belongs
        '''
        if self.name.endswith('ee'):
            return Channel.ee

        if self.name.endswith('mm'):
            return Channel.mm

        raise ValueError(f'Sample {self} does not belong to electron or muon Channel')
# ---------------------------------------
