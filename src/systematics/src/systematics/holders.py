'''
Module meant to contain classes used to store other objects
'''

from pydantic import BaseModel, ConfigDict
from typing   import TypeVar, Generic

T = TypeVar('T')
class ChannelHolder(BaseModel, Generic[T]):
    '''
    Class mean to hold objects per channel
    '''
    model_config = ConfigDict(arbitrary_types_allowed=True)

    ee : T 
    mm : T 
