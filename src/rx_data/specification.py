'''
Module holding Specification and Sample classes
'''

from pathlib   import Path
from pydantic  import BaseModel, ConfigDict

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
