'''
This module contains classes derived from Enum
'''
from enum   import IntEnum, StrEnum, auto
from typing import Literal

# ------------------------------
class Correction(StrEnum):
    '''
    Enum meant to symbolize corrections, e.g. PID
    '''
    pid = 'PID'
# ------------------------------
class Particle(StrEnum):
    '''
    Model meant to store particle properties
    '''
    kaon = 'kaon'
    pion = 'pion'
# ------------------------------
class Mass(StrEnum):
    '''
    Class meant to represent variables found in ROOT trees

    Attributes
    -----------------
    lb/b         : Lambda/B meson mass
    lb/b_dtf_jpsi: Lambda/B meson mass with DTF constraint on Jpsi mass
    b_bcor       : B meson mass with brem corrected electrons
    b_bcor_smr   : B meson mass with brem correction and smearing
    
    Attributes for MisID
    -----------------
    b_bcorr_pipi : B meson mass with electrons reconstructed as pions
    b_bcorr_kk   : B meson mass with electrons reconstructed as kaons
    '''

    lb_dtf_jpsi  = 'Lb_cons_Jpsi_M'
    lb           = 'Lb_M'

    bp           = 'B_M'
    bp_dtf_jpsi  = 'B_const_mass_M'
    bp_dtf_psi2  = 'B_const_mass_psi2S_M'
    bp_bcor      = 'B_Mass'
    bp_bcor_smr  = 'B_Mass_smr'
    bp_kk        = 'B_Mass_hdkk'
    bp_pipi      = 'B_Mass_hdpipi'

    bd           = 'B_M'
    bd_dtf_jpsi  = 'B_const_mass_M'
    bd_dtf_psi2  = 'B_const_mass_psi2S_M'
    bd_bcor      = 'B_Mass'
    bd_bcor_smr  = 'B_Mass_smr'
    bd_kk        = 'B_Mass_hdkk'
    bd_pipi      = 'B_Mass_hdpipi'
    # ------------------------------
    @property
    def latex(self) -> str:
        '''
        Latex string, for x-axis label
        '''
        match self:
            # ------------
            # Lambda_b
            # ------------
            case Mass.lb_dtf_jpsi:
                return r'$M_{DTF}^{J/\psi}(\Lambda_b)$'
            case Mass.lb:
                return r'$M(\Lambda_b)$'
            # ------------
            # B+
            # ------------
            case Mass.bp:
                return r'$M(B^+)$'
            case Mass.bp_dtf_jpsi:
                return r'$M_{DTF}^{J/\psi}(B^+)$'
            case Mass.bp_dtf_psi2:
                return r'$M_{DTF}^{\psi(2S)}(B^+)$'
            case Mass.bp_bcor:
                return r'$M_{corr}(B^+)$'
            case Mass.bp_bcor_smr:
                return r'$M_{corr}^{smr}(B^+)$'
            case Mass.bp_kk:
                return r'$M^{KK}(B^+)$'
            case Mass.bp_pipi:
                return r'$M^{\pi\pi}(B^+)$'
            # ------------
            # B0
            # ------------
            case Mass.bd:
                return r'$M(B^0)$'
            case Mass.bd_dtf_jpsi:
                return r'$M_{DTF}^{J/\psi}(B^0)$'
            case Mass.bd_dtf_psi2:
                return r'$M_{DTF}^{\psi(2S)}(B^0)$'
            case Mass.bd_bcor:
                return r'$M_{corr}(B^0)$'
            case Mass.bd_bcor_smr:
                return r'$M_{corr}^{smr}(B^0)$'
            case Mass.bd_kk:
                return r'$M^{KK}(B^0)$'
            case Mass.bd_pipi:
                return r'$M^{\pi\pi}(B^0)$'
    # ------------------------------
    @property
    def limits(self) -> tuple[float,float]:
        '''
        Range used for observable limits
        '''
        generic = 5000, 6000

        match self:
            # ------------
            # Lambda_b
            # ------------
            case Mass.lb_dtf_jpsi:
                return generic 
            case Mass.lb:
                return generic 
            # ------------
            # B+
            # ------------
            case Mass.bp:
                return 4500, 7000 
            case Mass.bp_dtf_jpsi:
                return 5080, 5680 
            case Mass.bp_dtf_psi2:
                return 5150, 5850 
            case Mass.bp_bcor:
                return Mass.bp.limits 
            case Mass.bp_bcor_smr:
                return Mass.bp.limits 
            case Mass.bp_kk:
                return 5000, 5800 
            case Mass.bp_pipi:
                return Mass.bp_kk.limits
            # ------------
            # B0
            # ------------
            case Mass.bd:
                return 4500, 7000 
            case Mass.bd_dtf_jpsi:
                return 5080, 5680 
            case Mass.bd_dtf_psi2:
                return 5150, 5850 
            case Mass.bd_bcor:
                return Mass.bd.limits
            case Mass.bd_bcor_smr:
                return Mass.bd.limits
            case Mass.bd_kk:
                return 5000, 5800 
            case Mass.bd_pipi:
                return Mass.bd_kk.limits
# ------------------------------
class Parameter(StrEnum):
    '''
    Class meant to store parameters of interest

    Attributes:

    rpk   : RpK
    irjpsi: Inverse of rjpsi
    '''
    rk     = 'rk'
    rx     = 'rx'
    rpk    = 'rpk'
    rkst   = 'rkst'

    rjpsi  = 'rjpsi'
    irjpsi = 'rjpsi_inv'
# ---------------------------------------
class Brem(IntEnum):
    '''
    Enum meant to represent brem category
    '''
    zero = 0 
    one  = 1 
    two  = 2 
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
    def channel(self) -> Channel:
        '''
        Either 'muon' or 'electron'
        '''
        if 'EE_MVA'            in self.value:
            return Channel.ee

        if 'EE_SameSign_MVA'   in self.value:
            return Channel.ee

        if 'MuMu_MVA'          in self.value:
            return Channel.mm
        
        if 'MuMu_SameSign_MVA' in self.value:
            return Channel.mm

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
class Component(StrEnum):
    '''
    Class meant to represent decay 

    Naming constraints:

    - Except for data, undefined and combinatorial, all samples meant to be used with the electron/muon channel should end with ee/mm
    '''
    undefined      = auto() 
    comb           = auto() 
    ccbar          = auto() 
    # -----
    # Data
    # -----
    data_24        = auto() 
    data_24_mu_c2  = auto() 
    data_24_md_c2  = auto() 
    data_24_mu_c3  = auto() 
    data_24_md_c3  = auto() 
    data_24_mu_c4  = auto() 
    data_24_md_c4  = auto() 
    # -----
    bpkkk          = auto() 
    bpkpipi        = auto() 
    # -----
    bpkpee         = auto() 
    bpkpjpsiee     = auto() 
    bpkppsi2ee     = auto() 
    bpkpipiee      = auto() 
    # -----
    bppijpsimm     = auto() 
    bppijpsiee     = auto() 
    # -----
    bpkpmm         = auto() 
    bpkpjpsimm     = auto() 
    bpkppsi2mm     = auto() 
    # -----
    bdkstkpiee     = auto() 
    bdkstkpijpsiee = auto() 
    bdkstkpipsi2ee = auto() 
    # -----
    bdkstkpimm     = auto() 
    bdkstkpijpsimm = auto() 
    bdkstkpipsi2mm = auto() 
    # -----
    bpk1kpipiee    = auto() 
    bpk2kpipiee    = auto() 
    bpkstkpiee     = auto() 
    bsphiee        = auto() 
    # -----
    bpjpsixee      = auto() 
    bdjpsixee      = auto() 
    bsjpsixee      = auto() 
    # -----
    bpjpsixmm      = auto() 
    bdjpsixmm      = auto() 
    bsjpsixmm      = auto() 
    # -----
    bsphijpsimm    = auto() 
    bsphijpsiee    = auto() 
    # -----
    lbpkjpsimm     = auto() 
    lbpkjpsiee     = auto() 
    # -----
    # Emulated
    # -----
    bskstjpsimm    = auto()
    bskstjpsiee    = auto()

    bdkstjpsimm_swp= auto()
    bdkstjpsiee_swp= auto()     
    # --------------------------------------------
    @property
    def sample(self) -> str:
        match self:
          case Component.data_24:
            return 'DATA_24*'
          case Component.data_24_mu_c2:
            return 'DATA_24_MagUp_24c2'
          case Component.data_24_md_c2:
            return 'DATA_24_MagDown_24c2'
          case Component.data_24_mu_c3:
            return 'DATA_24_MagUp_24c3'
          case Component.data_24_md_c3:
            return 'DATA_24_MagDown_24c3'
          case Component.data_24_mu_c4:
            return 'DATA_24_MagUp_24c4'
          case Component.data_24_md_c4:
            return 'DATA_24_MagDown_24c4'
          # -----
          case Component.bpkkk:
            return 'Bu_KplKplKmn_eq_sqDalitz_DPC'
          case Component.bpkpipi:
            return 'Bu_piplpimnKpl_eq_sqDalitz_DPC'
          # -----
          case Component.bpkpee:
            return 'Bu_Kee_eq_btosllball05_DPC'
          case Component.bpkpjpsiee:
            return 'Bu_JpsiK_ee_eq_DPC'
          case Component.bpkppsi2ee:
            return 'Bu_psi2SK_ee_eq_DPC'
          case Component.bpkpipiee:
            return 'Bu_Kpipiee_eq_DPC_LSFLAT'
          # -----
          case Component.bppijpsimm:
            return 'Bu_JpsiPi_mm_eq_DPC'
          case Component.bppijpsiee:
            return 'Bu_JpsiPi_ee_eq_DPC'
          # -----
          case Component.bpkpmm:
            return 'Bu_Kmumu_eq_btosllball05_DPC'
          case Component.bpkpjpsimm:
            return 'Bu_JpsiK_mm_eq_DPC'
          case Component.bpkppsi2mm:
            return 'Bu_psi2SK_mm_eq_DPC'
          # -----
          case Component.bdkstkpiee:
            return 'Bd_Kstee_eq_btosllball05_DPC'
          case Component.bdkstkpijpsiee:
            return 'Bd_JpsiKst_ee_eq_DPC'
          case Component.bdkstkpipsi2ee:
            return 'Bd_psi2SKst_ee_eq_DPC'
          # -----
          case Component.bdkstkpimm:
            return 'Bd_Kstmumu_eq_btosllball05_DPC'
          case Component.bdkstkpijpsimm:
            return 'Bd_JpsiKst_mm_eq_DPC'
          case Component.bdkstkpipsi2mm:
            return 'Bd_psi2SKst_mm_eq_DPC'
          # -----
          case Component.bpk1kpipiee:
            return 'Bu_K1ee_eq_DPC'
          case Component.bpk2kpipiee:
            return 'Bu_K2stee_Kpipi_eq_mK1430_DPC'
          case Component.bpkstkpiee:
            return 'Bu_Kstee_Kpi0_eq_btosllball05_DPC'
          case Component.bsphiee:
            return 'Bs_phiee_eq_Ball_DPC'
          # -----
          case Component.bpjpsixee:
            return 'Bu_JpsiX_ee_eq_JpsiInAcc'
          case Component.bdjpsixee:
            return 'Bd_JpsiX_ee_eq_JpsiInAcc'
          case Component.bsjpsixee:
            return 'Bs_JpsiX_ee_eq_JpsiInAcc'
          # -----
          case Component.bpjpsixmm:
            return 'Bu_JpsiX_mm_eq_JpsiInAcc'
          case Component.bdjpsixmm:
            return 'Bd_JpsiX_mm_eq_JpsiInAcc'
          case Component.bsjpsixmm:
            return 'Bs_JpsiX_mm_eq_JpsiInAcc'
          # -----
          case Component.bsphijpsimm:
            return 'Bs_Jpsiphi_mm_eq_CPV_update2012_DPC'
          case Component.bsphijpsiee:
            return 'Bs_Jpsiphi_ee_eq_CPV_update2012_DPC'
          # -----
          case Component.lbpkjpsimm:
            return 'Lb_JpsipK_mm_eq_phsp_DPC'
          case Component.lbpkjpsiee:
            return 'Lb_JpsipK_ee_eq_phsp_DPC'
          # -----
          # Emulated samples
          # -----
          case Component.bskstjpsimm:
            return 'Bs_JpsiKst_mm_eq_DPC'
          case Component.bskstjpsiee:
            return 'Bs_JpsiKst_ee_eq_DPC'
          case Component.bdkstjpsimm_swp:
            return 'Bd_JpsiKst_mm_had_swp'
          case Component.bdkstjpsiee_swp:
            return 'Bd_JpsiKst_ee_had_swp'
          case _:
            raise ValueError(f'No sample defined for: {self}')
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
    def get_mc_samples(cls) -> list['Component']:
        '''
        Returns
        ---------------
        List of MC samples known to analysis
        '''
        return [ sample for sample in cls if sample.has_mc() ] 
    # --------------------------------------------
    def has_mc(self) -> bool:
        if self in [Component.undefined, Component.comb, Component.data_24]:
            return False

        return True
    # --------------------------------------------
    def __str__(self):
        '''
        Returns
        ----------------
        String representing sample name, e.g. bpkpee 
        '''
        return self.name
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
    # --------------------------------------------
    @classmethod
    def inclusive(cls, channel : Channel) -> list['CCbarComponent']:
        '''
        Parameters
        --------------
        channel: E.g. ee, mm

        Returns
        --------------
        List of charmonium components
        '''
        if channel == Channel.ee:
            return [
                Component.bpjpsixee,
                Component.bdjpsixee,
                Component.bsjpsixee]

        if channel == Channel.mm:
            return [
                Component.bpjpsixmm,
                Component.bdjpsixmm,
                Component.bsjpsixmm]

        raise ValueError(f'Invalid channel: {channel}')
# ---------------------------------------
CCbarComponent = Literal[
    Component.bpjpsixee,
    Component.bdjpsixee,
    Component.bsjpsixee,
    # --------
    Component.bpjpsixmm,
    Component.bdjpsixmm,
    Component.bsjpsixmm]
