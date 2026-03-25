'''
Module holding classes needed
to configure tests
'''
from typing      import Self
from pydantic    import BaseModel, RootModel, Field
from dmu.generic import utilities as gut

# ---------------------------------------------------
class PackageConf(BaseModel):
    '''
    Class meant to represent testing configuration
    for single project
    '''
    path   : str
    splits : int = Field(gt = 0, lt = 20)
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
