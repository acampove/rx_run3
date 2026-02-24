'''
Module meant to hold pydantic models, data classes and enums
'''
from pathlib     import Path
from rx_common   import Particle
from dmu.generic import UnpackerModel

# ------------------------------
class MisIDSampleSplitting(UnpackerModel):    # Tested
    '''
    Configuration controlling how samples are created and
    split into control and signal region
    '''
    branches      : list[str]
    tracks        : dict[str,str]
    hadron_tagging: dict[Particle,str]
    lepton_tagging: dict[str,str]
# ------------------------------
class MisIDSampleWeights(UnpackerModel):       # Tested
    '''
    Configuration controlling how to pick weights for misID samples
    '''
    maps_path : Path
    regions   : dict[str,str]
    pars      : tuple[str,str]
    splitting : MisIDSampleSplitting
# ------------------------------
