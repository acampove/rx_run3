'''
Module holding classes needed
to configure tests
'''
from pydantic import BaseModel, Field

# ---------------------------------------------------
class PackageConf(BaseModel):
    '''
    Class meant to represent testing configuration
    for single project
    '''
    path   : str
    splits : int = Field(gt = 0, lt = 20)
# ---------------------------------------------------
class TestConfig(BaseModel):
    '''
    Class meant to represent testing configurations
    '''
    packages : dict[str,PackageConf]
# ---------------------------------------------------
