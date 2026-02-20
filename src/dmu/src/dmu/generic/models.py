'''
Module meant to contain code needed to deal with loading of data into
pydantic models
'''

import yaml
import contextlib

from importlib.resources import files
from dmu                 import LogStore
from typing              import Self
from pathlib             import Path
from pydantic            import BaseModel, model_validator, ConfigDict

log=LogStore.add_logger('dmu:generic:models')

_PACKAGE : str | None = None
# --------------------------------------------------
class UnpackerModel(BaseModel):
    '''
    Class meant to allow derived classes to:

    - Load models from paths when the value is a string or path ending with '.yaml'
    - Load data directly from path to file
    '''
    model_config = ConfigDict(frozen=True)
    # --------------
    @staticmethod
    def _update_path(path : Path) -> Path:
        '''
        Parameters
        ---------------
        package: Name of data package, relative to which path is used
        path   : Relative path

        Returns
        ---------------
        Full path to `path`, i.e. this method updates relative paths
        '''
        if _PACKAGE is None:
            log.info(f'Not updating path: {path}')
            return path

        pkg_path = str(files(_PACKAGE))

        log.info(f'Updating path with {pkg_path}')
        full_path = pkg_path / path

        return full_path
    # --------------
    @staticmethod
    def _data_from_path(path : Path) -> dict:
        path = UnpackerModel._update_path(path = path)
        if not path.exists():
            raise ValueError(f'File not found: {path}')

        with open(path) as f:
            log.info(f'Loading from: {path}')
            loaded_data = yaml.safe_load(stream = f)

        return loaded_data
    # --------------
    @model_validator(mode='before')
    @classmethod
    def _unpack_paths(cls, data):
        '''
        This method will update data by unpacking yaml file paths
        and inserting the python data in the `data` variable
        '''
        for field_name, field_info in cls.model_fields.items():
            val = data.get(field_name)

            if not isinstance(val, (str, Path)):
                continue

            if not str(val).endswith('.yaml'):
                continue

            path        = Path(val)
            loaded_data = cls._data_from_path(path = path)
            FieldType   = field_info.annotation

            if FieldType is None:
                raise ValueError(f'Annotation not found for: {field_name}')

            data[field_name] = FieldType(**loaded_data)

        return data
    # --------------
    @classmethod
    def from_yaml(
        cls, 
        path    : Path | str,
        package : None | str = None) -> Self:
        '''
        Parameters
        ------------------
        path   : Path to yaml file storing configuration
        package: If used, this is the data package with respect to which YAML paths
                 will be found in order to unpack them

        Returns
        ------------------
        Instance of model
        '''
        if package is None:
            path = Path(path)
        else:
            path = Path(str(files(package))) / path

        if not path.exists():
            raise ValueError(f'Cannot find: {path}')

        with open(path) as ifile:
            data = yaml.safe_load(ifile)

        if package is None:
            return cls(**data)
        
        with cls.package(name = package):
            return cls(**data)
    # --------------
    def to_yaml(self, path : Path) -> None:
        '''
        Parameters
        ----------------
        path: Path to YAML file where current object will be saved
        '''
        py_data = self.model_dump()
        ym_data = yaml.safe_dump(data = py_data, indent = 2)
        path.write_text(ym_data)
    # --------------
    @classmethod
    def package(cls, name : str):
        '''
        Parameters
        -------------
        Package: name of package where YAML files will be searched
        '''
        global _PACKAGE
        old_val  = _PACKAGE
        _PACKAGE = name 

        @contextlib.contextmanager
        def _context():
            try:
                yield
            finally:
                global _PACKAGE
                _PACKAGE = old_val

        return _context()
# --------------------------------------------------
