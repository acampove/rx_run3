'''
Class holding FitParametersConf and classes needed by it
'''
from pydantic import BaseModel, Field, RootModel

# ----------------------
class GraphConf(BaseModel):
    '''
    Class meant to represent config for a single graph
    '''
    error : str
    xvals : list[str | int]
    xname : str
    yrange: list[float] = Field(min_length=2, max_length=2)
    label : str
    color : str
    xlabel: str
    ylabel: str
# ----------------------
class PlotConf(BaseModel):
    '''
    Class meant to hold configuration for single plot
    '''
    cuts : str
    plots: dict[str, GraphConf]
# ----------------------
class FitParametersConf(RootModel[dict[str,PlotConf]]):
    '''
    Class meant to hold parameters needed for plotting
    '''
    pass
