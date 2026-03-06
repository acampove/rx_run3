'''
Module meant to hold pydantic models, data classes and enums
'''
from pathlib     import Path
from rx_common   import Particle
from pydantic    import ConfigDict
from dmu.generic import UnpackerModel

# ------------------------------
class MisIDSampleSplitting(UnpackerModel):    # Tested
    '''
    Configuration controlling how samples are created and
    split into control and signal region
    '''
    model_config = ConfigDict(frozen=True)

    branches      : list[str]
    tracks        : dict[str,str]
    hadron_tagging: dict[Particle,str]
    lepton_tagging: dict[str,str]
# ------------------------------
class MisIDSampleWeights(UnpackerModel):       # Tested
    '''
    Configuration controlling how to pick weights for misID samples
    '''
    model_config = ConfigDict(frozen=True)

    maps_path : Path
    regions   : dict[str,str]
    pars      : tuple[str,str]
    splitting : MisIDSampleSplitting
# ------------------------------
