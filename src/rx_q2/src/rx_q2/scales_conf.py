'''
Module containing ScalesConf class
'''

from pydantic   import BaseModel, ConfigDict
from pathlib    import Path
from dmu        import LogStore

log=LogStore.add_logger('fitter:scales_conf')
#-------------------------------------
class ScalesConf(BaseModel):
    '''
    Class used to store configuration
    needed to extract mass scales and resolutions
    '''
    model_config = ConfigDict(frozen=True)
    jpsi_mass : float

    mm      : dict[str,list[float]]
    ee      : dict[str,list[float]]

    kind    : str # q2, B
    vers    : str
    year    : str
    proj    : str

    inp_dir : Path
    out_dir : Path
    regex   : str
    # ------------------------------
    def get_range(self, var : str) -> tuple[float,float]:
        '''
        Parameters
        -----------------
        var: Variable name, e.g. mu, sg, dm, ss

        Returns
        -----------------
        Y axis range for plot
        '''
        if   self.proj.endswith('_mm'):
            data = self.mm
        elif self.proj.endswith('_ee'):
            data = self.ee
        else:
            raise ValueError(f'Invalid project: {self.proj}')

        if var not in data:
            for value in data:
                log.error(value)
            raise ValueError(f'Invalid variable: {var}')

        [low, high] = data[var]

        return low, high
#-------------------------------------
