'''
This module contains classes derived from Enum
'''
from enum   import IntEnum, StrEnum
from .mass  import Mass

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
class MisID(StrEnum):
    '''
    Class meant to represent different
    misID control regions
    '''
    bp_kk   = 'bp_kk'
    bp_pipi = 'bp_pipi'

    bd_kk   = 'bd_kk'
    bd_pipi = 'bd_pipi'
    # ----------------------------
    def __str__(self):
        return self.value
    # ----------------------------
    @property
    def mass(self) -> Mass:
        '''
        Mass meant to be fitted in this region
        '''
        match self:
            case MisID.bp_kk:
                return Mass.bp_kk
            case MisID.bp_pipi:
                return Mass.bp_pipi
            case MisID.bd_kk:
                return Mass.bd_kk
            case MisID.bd_pipi:
                return Mass.bd_pipi
# ---------------------------------------

