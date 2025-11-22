'''
Module holding code meant to be reused elsewhere
'''

from importlib.resources import files

from torch               import Tensor
from dask                import dataframe
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
    config_path = str(config_path)
    data        = gut.load_json(config_path)

    return data
# ---------------------------------------------
def _inject_bias(ddf : DDF, bias : float, kind : str) -> DDF:
    '''
    This function scales the momentum components of the lepton by the `bias` factor
    This is done only when the electrons have brem associated, i.e. L*_brem == 1

    Parameters
    --------------
    kind (str) : Type of bias
        flat: bias is uncorrelated with anything
        row : Correlation with the row
    '''
    for lep in ['L1', 'L2']:
        if   kind == 'flat':
            ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - 1)
        elif kind == 'row' :
            ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - ddf[f'{lep}_BREMHYPOROW'] / 60.)
        elif kind == 'row_col' :
            ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - ddf[f'{lep}_BREMHYPOROW'] / 60. * ddf[f'{lep}_BREMHYPOCOL']/ 60.)
        elif kind == 'row_col_are' :
            ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - ddf[f'{lep}_BREMHYPOROW'] / 60. * ddf[f'{lep}_BREMHYPOCOL']/ 60.) * ddf[f'{lep}_BREMHYPOAREA']
        elif kind == 'row_col_are_eng' :
            ddf[f'{lep}_PT'] = ddf[f'{lep}_PT'] + ddf[f'{lep}_PT'] * ddf[f'{lep}_brem'] * (bias - ddf[f'{lep}_BREMHYPOROW'] / 60. * ddf[f'{lep}_BREMHYPOCOL']/ 60.) * ddf[f'{lep}_BREMHYPOAREA'] * ddf[f'{lep}_BREMTRACKBASEDENERGY'] / 10_000.
        else:
            raise ValueError(f'Invalid bias: {kind}')

    return ddf
# ------------------------------------
def normalize_tensor(x : Tensor) -> Tensor:
    '''
    Makes sure the mean of the distribution is zero and the standard deviation is 1
    '''
    mean = x.mean()
    std  = x.std()
    x    = (x - mean) / std

    return x
# ------------------------------------
def get_ddf(
        name : str,
        bias : float|None,
        kind : str) -> DDF:
    '''
    Returns Dask DataFrame with toy data, used for tests

    Parameters
    ---------------
    name (str)  : Name of file, e.g. real_data/fake_data
    bias (float): Numerical value of bias, if flat, should be around 1, 1 will be no bias. Pass None, if no bias is will be used
    kind (str)  : Type of bias, `flat` for same bias for all electrons, `row` for row dependent one.
    '''
    data_path = files('ecal_calibration_data').joinpath(f'tests/data/{name}.parquet')
    data_path = str(data_path)
    ddf       = dataframe.read_parquet(data_path)

    if bias is None:
        return ddf

    ddf = _inject_bias(ddf=ddf, bias=bias, kind=kind)

    return ddf
# ------------------------------------
