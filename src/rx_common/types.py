'''
This module contains classes derived from Enum
'''

from enum import Enum

class Trigger(str, Enum):
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

class Channel(str, Enum):
    '''
    This class repsesents the electron, muon or emu channel types
    '''
    ee = 'ee'
    mm = 'mm'
    em = 'em'

    def __str__(self):
        return self.value


class Project(str, Enum):
    '''
    This class represents the projects
    '''
    rk            = 'rk'
    rk_no_refit   = 'rk_no_refit'
    rk_sim10d     = 'rk_sim10d'
    # ---------------
    rkst          = 'rkst'
    rkst_no_refit = 'rkst_no_refit'
    rkst_sim10d   = 'rkst_sim10d'

    def __str__(self):
        return self.value

class Qsq(str, Enum):
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

