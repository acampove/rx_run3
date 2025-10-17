'''
Module containing Conf class
'''
import os
from pathlib        import Path
from typing         import Any

from dmu.stats.zfit import zfit
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
    nent      : int
    year      : str
    trigger   : str
    brem      : int
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
    model     : dict[str,list[int]]
    simulation: MCFit
#-------------------
class Config(BaseModel):
    '''
    Class meant to hold configuration
    '''
    zfit.settings.changed_warnings.hesse_name = False
    ana_dir  = Path(os.environ['ANADIR'])

    cfg_vers : str
    syst     : str
    input    : Input
    fitting  : Fitting
