'''
Module containing Conf class
'''
import os
from typing         import Self
from pathlib        import Path
from typing         import Any
from pydantic       import BaseModel, computed_field, model_validator, ConfigDict

from dmu.stats.zfit        import zfit
from zfit.interface        import ZfitSpace as zobs
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
    def __getitem__(self, name : str) -> str:
        if not hasattr(self, name):
            raise AttributeError(f'Samples class has no {name} attribute')

        return getattr(self, name)
#-------------------
class Input(BaseModel):
    '''
    Class meant to represent input section of config
    '''
    nentries  : int
    year      : str
    trigger   : str
    brem      : int
    block     : str
    kind      : str     # dat or sim, used to pick from Samples below
    samples   : Samples
    selection : dict[str,str]
#-------------------
class Plotting(BaseModel):
    nbins : int
#-------------------
class FitModel(BaseModel):
    '''
    Class meant to represent a fitting model
    '''
    pdfs : list[str]
#-------------------
class Fitting(BaseModel):
    '''
    Class used to store fitting configuration
    '''
    ranges    : dict[int,list[float]]
    mass      : str
    weights   : str
    plotting  : Plotting 
    model     : FitModel
    simulation: MCFit
    skip      : bool
#-------------------
class Config(BaseModel):
    '''
    Class meant to hold configuration
    '''
    model_config = ConfigDict(
        arbitrary_types_allowed = True,
        frozen                  = True)

    logl     : int
    ana_dir  : Path
    vers     : str
    syst     : str
    input    : Input
    fitting  : Fitting
    #-------------------
    @model_validator(mode='after')
    def _update_selection(self) -> Self:
        '''
        Updates input.selection dictionary
        '''
        self.input.selection['block'] = self._get_block_cut()

        return self
    #-------------------
    @computed_field
    @property
    def obs_range(self) -> tuple[float,float]:
        brem        = self.input.brem
        rng         = self.fitting.ranges[brem]
        [low, high] = rng

        return low, high
    #-------------------
    def _get_block_cut(self) -> str:
        block = self.input.block

        if block == 'all':
            return '(1)'

        if block == '12':
            return '(block == 1) || (block == 2)'

        if block == '78':
            return '(block == 7) || (block == 8)'

        return f'block == {block}'
    #-------------------
    @computed_field
    @property
    def out_dir(self) -> Path:
        ana_dir = Path(os.environ['ANADIR'])
        path_1  = f'q2/fits/{self.vers}/{self.input.kind}'
        path_2  = f'{self.input.trigger}_{self.input.year}'
        path_3  = f'{self.input.brem}_{self.input.block}_{self.syst}'

        out_dir = ana_dir / path_1 / path_2 / path_3
        out_dir.mkdir(parents=True, exist_ok=True)

        return out_dir
    #-------------------
    @computed_field
    @property
    def obs(self) -> zobs:
        '''
        Observable used in fit
        '''
        name = self.fitting.mass

        return zfit.Space(name, limits=self.obs_range)
#-------------------
