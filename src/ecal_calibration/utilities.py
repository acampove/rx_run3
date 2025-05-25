'''
Module holding code meant to be reused elsewhere
'''

from importlib.resources import files

from dask.dataframe      import DataFrame as DDF
import dmu.generic.utilities as gut

# ------------------------------------
def load_cfg(name : str) -> dict:
    '''
    Loads YAML file with configuration and returns dictionary.

    Parameters:

    name (str) : String representing part of the path to config file, e.g. regressor/simple for .../regressor/simple.yaml
    '''

    config_path = files('ecal_calibration_data').joinpath(f'{name}.yaml')
    data        = gut.load_json(config_path)

    return data
# ------------------------------------
def get_ddf() -> DDF:
    '''
    Returns Dask DataFrame with toy data, used for tests
    '''
# ------------------------------------
