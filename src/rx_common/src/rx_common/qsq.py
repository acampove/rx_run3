'''
Module containing Qsq enum
'''
from enum      import StrEnum

class Qsq(StrEnum):
    '''
    This class represents different q2 bins
    '''
    none    = 'none'
    all     = 'all'
    low     = 'low'
    central = 'central'
    jpsi    = 'jpsi'
    psi2    = 'psi2'
    high    = 'high'
    # --------------------------
    @property
    def kind(self) -> str:
        '''
        E.g. rare or reso
        '''
        if self.is_rare:
            return 'rare'
        
        return 'reso'
    # --------------------------
    @property
    def is_rare(self) -> bool:
        if self in [Qsq.low, Qsq.central, Qsq.high]:
            return True

        return False
    # --------------------------
    @property
    def latex(self) -> str:
        '''
        Returns
        --------------
        Latex representation of this bin
        needed for plots 
        '''
        match self:
            case Qsq.low:
                return 'Low' 
            case Qsq.central:
                return 'Central' 
            case Qsq.jpsi:
                return r'$J/\psi$' 
            case Qsq.psi2:
                return r'$\psi(2S)$' 
            case Qsq.high:
                return 'High' 
            case Qsq.all:
                return 'All'
            case Qsq.none:
                return 'None'
    # --------------------------
    def __str__(self):
        return self.value

