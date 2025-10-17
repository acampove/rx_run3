'''
Module containing Conf class
'''
from pathlib        import Path
from typing         import Any

from pydantic       import BaseModel

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_q2:config')
#-------------------
class MCFit(BaseModel):
    '''
    Class holding configuration needed for MC fits
    '''
    strategy : dict[str,dict[str,Any]]
#-------------------
class Samples(BaseModel):
    '''
    Class meant to represent sample information
    '''
    sim : str
    dat : str
#-------------------
class Input(BaseModel):
    '''
    Class meant to represent input section of config
    '''
    nentries  : int
    year      : str
    trigger   : str
    brem      : int
    kind      : str     # dat or sim, used to pick from Samples below
    samples   : Samples
    selection : dict[str,str]
#-------------------
class Fitting(BaseModel):
    '''
    Class used to store fitting configuration
    '''
    ranges    : dict[int,list[int]]
    mass      : str
    weights   : str
    binning   : dict[str,int]
    model     : dict[str,list[str]]
    simulation: MCFit
#-------------------
class Config(BaseModel):
    '''
    Class meant to hold configuration
    '''
    ana_dir  : Path
    vers     : str
    syst     : str
    input    : Input
    fitting  : Fitting
