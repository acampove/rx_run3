'''
Module holding classes needed
to configure tests
'''
from pathlib     import Path
from typing      import Self
from pydantic    import BaseModel, RootModel, Field, field_validator
from dmu.generic import utilities as gut
from importlib   import util      as iut

# ---------------------------------------------------
class PackageConf(BaseModel):
    '''
    Class meant to represent testing configuration
    for single project
    '''
    path   : str 
    splits : int = Field(gt = 0, lt = 50)
    # --------------------------------
    @field_validator('path', mode = 'before')
    @classmethod
    def find_path(cls, path : str) -> str:
        '''
        Parameters
        ------------
        path: Package name 

        Returns
        ------------
        Full path to tests/ directory
        '''
        spec = iut.find_spec(path)
        if spec is None:
            raise ValueError(f'Cannot find path for package: {path}')

        origin = spec.origin
        if origin is None:
            raise ValueError(f'Path to init not found in: {spec}')

        fpath = Path(origin)
        if not fpath.exists():
            raise ValueError(f'Cannot find: {fpath}')

        test_path = fpath.parent.parent.parent / 'tests'
        if not test_path.exists():
            raise ValueError(f'Cannot find: {test_path}')

        return str(test_path)
# ---------------------------------------------------
class TestConfig(RootModel[dict[str,PackageConf]]):
    '''
    Class meant to represent testing configurations
    '''
    # --------------------------------
    def __getitem__(self, key: str) -> PackageConf:
        return self.root[key]

    def __getattr__(self, key: str) -> PackageConf:
        return self.root[key]

    def __iter__(self):
        yield from self.root.items()

    def __contains__(self, key: str) -> bool:
        return key in self.root

    def items(self):
        return self.root.items()

    def keys(self):
        return self.root.keys()

    def values(self):
        return self.root.values()
    # --------------------------------
    @classmethod
    def from_package(cls, file_path : str, package : str) -> Self:
        '''
        Parameters
        ---------------
        file_path: Path to config, wrt package, e.g. local/config.yaml
        '''
        data = gut.load_data(package = package, fpath = file_path)

        return cls(**data)
# ---------------------------------------------------
