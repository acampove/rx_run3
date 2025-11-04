'''
This module contains classes derived from Enum
'''

from enum import Enum

class Trigger(str, Enum):
    '''
    Class meant to represent MVA triggers
    '''
    rk_ee_os   = 'Hlt2RD_BuToKpEE_MVA'
    rk_mm_os   = 'Hlt2RD_BuToKpMuMu_MVA'
    # -----------
    rkst_ee_os = 'Hlt2RD_B0ToKpPimEE_MVA'
    rkst_mm_os = 'Hlt2RD_B0ToKpPimMuMu_MVA'

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
    rkst          = 'rkst'
    rk_no_refit   = 'rk_no_refit'
    rkst_no_refit = 'rkst_no_refit'

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

