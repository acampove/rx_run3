'''
Module containing classes representing configurations used for fitting
'''
import dataclasses

from functools   import cached_property
from pathlib     import Path
from typing      import Literal
from dmu.generic import UnpackerModel
from rx_common   import Brem, Mass, Project, Qsq, Particle
from rx_common   import Component, Trigger, CCbarComponent
from pydantic    import BaseModel, ConfigDict
from dmu         import LogLevels, LogStore
from dmu.stats   import ModelFactoryConf
from dmu.stats   import KDEConf
from dmu.stats   import FitConf
from dmu.stats   import ZFitPlotterConf 
from dmu.stats   import Model as FModel
from dmu.stats   import zfit
from zfit        import Space      as zobs
from .types      import CCbarWeight
from .toy_maker  import ToyConf

log=LogStore.add_logger('fitter:configs')
# ------------------------------
# Components
# ------------------------------
class ComponentConf(BaseModel):         # Tested
    '''
    Class meant to configure the fit for different components
    '''
    model_config = ConfigDict(frozen=True)

    output_directory : Path
# ------------------------------
class CmbConstraintConf(BaseModel):
    '''
    Class meant to represent configuration for combintorial constraints

    Attributes
    ---------------
    selection : Selection that sample will go through on top of nominal selection, e.g. vetos
    parameters: Parameters that will be constrained
    '''
    sample    : Component
    trigger   : Trigger
    parameters: list[str]
    selection : dict[str,str]
    fit       : FitConf
    plots     : ZFitPlotterConf
# ------------------------------
class CombinatorialConf(ComponentConf, UnpackerModel): # Tested
    '''
    Class mean to control configuration of fit to combinatorial
    '''
    model_config = ConfigDict(frozen=True)

    models     : dict[Qsq, ModelFactoryConf ]
    constraints: dict[Qsq, CmbConstraintConf]
# ------------------------------
class NonParametricConf(ComponentConf): # Tested
    '''
    Class mean to control configuration of fit to non parametric models 
    '''
    sample : Component
    fit    : KDEConf
    plots  : ZFitPlotterConf 
# ------------------------------
class BremCatConf(BaseModel):  # Tested
    '''
    Class meant to configure a brem category fit
    '''
    selection : dict[str,str]
    model     : list[FModel ]
# ------------------------------
class ParametricConf(ComponentConf):  # Tested
    '''
    Class mean to control configuration of fit to parametric models 
    '''
    sample     : Component
    fit        : FitConf 
    categories : dict[Brem, BremCatConf]
    plots      : ZFitPlotterConf 
# ------------------------------
class CCbarConf(ComponentConf):         # Tested
    '''
    Class mean to control configuration to fit ccbar components
    '''
    model_config = ConfigDict(frozen=True)

    ccbar_samples : list[CCbarComponent]
    weights       : dict[CCbarWeight,bool]
    fitting       : KDEConf
# ------------------------------
# MisID
# ------------------------------
class MisIDSampleWeights(UnpackerModel):       # Tested
    '''
    Configuration controlling how to pick weights for misID samples
    '''
    maps_path : Path
    regions   : dict[str,str]
    pars      : tuple[str,str]
# ------------------------------
class MisIDSampleSplitting(UnpackerModel):    # Tested
    '''
    Configuration controlling how samples are split into control and
    signal region
    '''
    branches      : list[str]
    tracks        : dict[str,str]
    hadron_tagging: dict[Particle,str]
    lepton_tagging: dict[str,str]
# ------------------------------
class MisIDCategory(UnpackerModel):          # Tested
    selection  : dict[str,str]
    weights    : MisIDSampleWeights 
    splitting  : MisIDSampleSplitting 
# ------------------------------
class MisIDConf(UnpackerModel, NonParametricConf): # Tested
    '''
    Configuration needed to build MisID components
    '''
    trigger    : Trigger
    project    : Project
    categories : dict[str,MisIDCategory]
# ------------------------------
# Yields, observables, etc
# ------------------------------
class YieldConf(BaseModel):            # Tested
    '''
    Class representing configuration for yields
    '''
    val    : float
    min    : float
    max    : float
    scl    : list[str]         | None = None
    prefix : Literal['pscale'] | None = None
# ------------------------------
class ObservableConf(BaseModel):
    '''
    Class representing configuration for observable
    '''
    model_config = ConfigDict(frozen=True)

    name : Mass
    range: tuple[int,int]
# ------------------------------
class MisIDFitComponents(UnpackerModel):
    '''
    Class describing fitting components for fits to misID control region
    '''
    model_config = ConfigDict(frozen=True)

    cominatorial : CombinatorialConf 
    hdpipi       : NonParametricConf
    hdkk         : NonParametricConf
# ------------------------------
class MisIDFitModel(UnpackerModel):
    '''
    Class meant to configure how the fit model for misID control region
    fits are done
    '''
    model_config = ConfigDict(frozen=True)

    yields     : dict[str,YieldConf]
    observable : dict[str,ObservableConf]
    components : MisIDFitComponents
# ------------------------------
class MisIDConstraintConf(UnpackerModel):
    '''
    Configuration meant to control how fits used to get misID constraints
    are used to get the constraints
    '''
    model_config = ConfigDict(frozen=True)

    output_directory : Path
    trigger          : Trigger
    selection        : dict[str,str]
    fit              : FitConf
    plots            : ZFitPlotterConf
    model            : MisIDFitModel 
# ------------------------------
class ConstraintsCfg(UnpackerModel):
    '''
    Class meant to configure constraints
    '''
    model_config = ConfigDict(frozen=True)

    misid   : MisIDConstraintConf | None = None
    pre_rare: list[Component]     | None = None
# ------------------------------
AnyModel = CombinatorialConf | ParametricConf
# ------------------------------
# Fits
# ------------------------------
class FitModelConf(UnpackerModel):
    '''
    Class representing fitting model
    '''
    model_config = ConfigDict(frozen = True)

    yields     : dict[str,YieldConf]
    observable : dict[Qsq, ObservableConf]
    components : dict[Component, CombinatorialConf]
    constraints: ConstraintsCfg
    plots      : ZFitPlotterConf
    fit        : FitConf
# ------------------------------
class DataModelConf(UnpackerModel):
    '''
    Configuration needed to do a fit to data
    '''
    model_config = ConfigDict(frozen=True)
    
    trigger          : Trigger
    output_directory : Path
    model            : FitModelConf
# ------------------------------
