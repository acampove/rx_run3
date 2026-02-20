'''
Module meant to contain code needed to deal with loading of data into
pydantic models
'''

import yaml

from importlib.resources import files
from typing              import Self
from pathlib             import Path
from pydantic            import BaseModel, model_validator, ConfigDict

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
    def _update_path(path : Path, package : str) -> Path:
        '''
        Parameters
        ---------------
        package: Name of data package, relative to which path is used
        path   : Relative path

        Returns
        ---------------
        Full path to `path`, i.e. this method updates relative paths
        '''
        pkg_path = str(files(package))

        full_path = pkg_path / path
        if not full_path.exists():
            raise ValueError(f'Cannot find: {full_path}')

        return full_path
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

            path = Path(val)

            if 'package' in data:
                path = cls._update_path(
                    path    = path, 
                    package = data['package'])

            if not path.exists():
                raise ValueError(f'File not found: {path}')

            annotation = field_info.annotation
            if annotation is None:
                raise ValueError(f'Annotation not found for: {field_name}')

            with open(path) as f:
                loaded_data = yaml.safe_load(stream = f)

            data[field_name] = annotation(**loaded_data)

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

        data['package'] = package
        
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
# --------------------------------------------------
