'''
Module with Region enum
'''

from enum       import StrEnum, auto
from dmu.stats  import zfit

from .mass      import Mass
from .component import Component

# ------------------------
class Region(StrEnum):
    '''
    Enum meant to symbolize control and signal regions, 
    e.g. hadronic misID control region
    '''
    # ---------------------
    bpkk   = auto()
    bppipi = auto()
    # ---------------------
    @property
    def signal(self) -> Component:
        '''
        Signal component for this region
        '''
        match self:
            case Region.bpkk:
                return Component.bpkkk
            case Region.bppipi:
                return Component.bpkpipi
    # ---------------------
    @property
    def obs(self) -> zfit.Space:
        '''
        Observable for corresponding region
        '''
        obs = zfit.Space(
            obs    = self.mass, 
            limits = self.mass.limits)

        return obs
    # ---------------------
    @property
    def mass(self) -> Mass:
        '''
        Mass that should be fitted in region
        '''
        match self:
            case Region.bpkk:
                return Mass.bp_kk
            case Region.bppipi:
                return Mass.bp_pipi
    # ---------------------
    @staticmethod
    def hadronic_misid() -> list['Region']:
        '''
        Returns list of hadronic misid regions
        '''
        return [Region.bpkk, Region.bppipi]
# ------------------------
