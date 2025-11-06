'''
Module with SpecMaker class
'''
import os
import yaml
import copy
import secrets
import fnmatch
import pprint

from pydantic              import BaseModel, ConfigDict
from contextlib            import contextmanager
from omegaconf             import DictConfig, OmegaConf
from pathlib               import Path
from typing                import overload, Literal, Final
from rx_common.types       import Trigger
from rx_common             import info
from rx_data.path_splitter import PathSplitter, NestedSamples
from dmu.logging.log_store import LogStore
from dmu.generic           import hashing
from dmu.generic           import version_management as vmn
from dmu.generic           import utilities          as gut

log=LogStore.add_logger('rx_data:spec_maker')
# --------------------------
class Sample(BaseModel):
    '''
    Class meant to represent a sample
    '''
    files : list[Path]
    trees : list[str]
# --------------------------
class Specification(BaseModel):
    '''
    Class meant to represent a specification needed
    to build ROOT dataframes
    '''
    friends : dict[str,Sample]
    samples : dict[str,Sample]

    model_config = ConfigDict(frozen=True)
# --------------------------
class SpecMaker:
    '''
    Class meant to:

    - Find samples and use them to create a JSON file with them
    - Save file and make path available to user
    '''
    # ----------------------
    def __init__(self, sample : str, trigger : Trigger) -> None:
        '''
        Parameters
        -------------
        sample : E.g. Bu_JpsiK_ee_eq_DPC
        trigger: Hlt2RD_BuToKpEE_MVA
        '''
        self._sample = sample
        self._trigger= trigger
    # ----------------------
    def get_path(self) -> Path:
        '''
        Returns
        ------------
        Path to JSON file with specification
        '''
        path = Path('/tmp/file.json')
        path.touch(exist_ok=True)

        return path
# --------------------------
