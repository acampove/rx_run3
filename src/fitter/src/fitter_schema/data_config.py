'''
This file defines the schema that all the data.yaml
configs need to follow
'''

from dataclasses import dataclass, field
from omegaconf   import MISSING
from typing      import Any

@dataclass
class ObservableConfig:
    name : str         = MISSING
    range: list[float] = field(default_factory=list)

@dataclass
class YRangeConf:
    log    : list[Any] = field(default_factory=list[Any]) 
    linear : list[Any] = field(default_factory=list[Any]) 

@dataclass
class PlotConfig:
    nbins  : int            = 30
    stacked: bool           = False
    yrange : YRangeConf     = field(default_factory=YRangeConf)
    d_leg  : dict[str, str] = field(default_factory=dict)
    plot_components : str   = field(default_factory=str)

@dataclass
class ModelConfig:
    yields     : str                         = MISSING
    observable : dict[str, ObservableConfig] = MISSING
    components : dict[str, str]              = field(default_factory=dict)
    constraints: dict[str, Any]              = field(default_factory=dict)

@dataclass
class ConfigSchema:
    output_directory: str            = MISSING
    model           : ModelConfig    = MISSING
    fit             : dict[str, Any] = field(default_factory=dict)   # If unspecified now
    plots           : PlotConfig     = field(default_factory=PlotConfig)
    trigger         : str            = field(default_factory=str)
    selection       : dict[str, str] = field(default_factory=dict)
