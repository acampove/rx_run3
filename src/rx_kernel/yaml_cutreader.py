'''
Module with code to interface with YamlCutReader c++ class
'''
# pylint: disable=invalid-name, import-error

from typing import Union
from ROOT   import YamlCutReader as YamlCutReader_cpp

# -----------------------------------------------------------
def YamlCutReader(yaml_path : Union[str,None] = None) -> YamlCutReader_cpp:
    '''
    Function returning YamlCutReader c++ implementation's instance
    '''
    if yaml_path is None:
        return YamlCutReader_cpp()

    obj = YamlCutReader_cpp(yaml_path)

    return obj
# -----------------------------------------------------------
