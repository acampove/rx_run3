'''
Module meant to hold functions to test the ParsLoader class
'''
import pytest
import pandas as pnd

from typing        import Literal
from rx_common     import Parameter
from dmu           import LogStore
from fitter        import ParsLoader

log     = LogStore.add_logger('stats:test_pars_loader')
Channel = Literal['ee', 'mm']
VERSION = 'v23/L0E'
POI     = Parameter.irjpsi
# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_stats:pars_loader', 10)
# ----------------------
@pytest.mark.skip(reason = 'Inputs missing')
def test_syst() -> None:
    '''
    Test retriving dataframe for given systematic
    '''
    ldr = ParsLoader(name = VERSION)
    df  = ldr.df_by_sys(sys = 'nominal')

    print(df)

    assert isinstance(df, pnd.DataFrame)
    assert set(df.columns.to_list()) == {'Parameter', 'Value', 'Error', 'Gen', 'Toy', 'GOF', 'Valid'}
    assert len(df) > 0
# ----------------------
@pytest.mark.skip(reason = 'Inputs missing')
def test_poi() -> None:
    '''
    Test loading dataframe with POI
    '''
    ldr = ParsLoader(name = VERSION)
    df  = ldr.df_by_poi(poi = POI)

    print(df)

    assert isinstance(df, pnd.DataFrame)
    assert len(df) > 0
# ----------------------
@pytest.mark.skip(reason = 'Inputs missing')
def test_fix() -> None:
    '''
    Test loading dataframe with POI information
    as well as last parameter that was fixed
    '''
    ldr = ParsLoader(name = VERSION)
    df  = ldr.df_by_fix(poi = POI)

    nfixed = df['nFixed'].unique().tolist()

    print(df)

    assert len(nfixed) > 0
    assert isinstance(df, pnd.DataFrame)
    assert set(df.columns.to_list()) == {'Value', 'Error', 'Gen', 'Fixed', 'nFixed', 'Valid'}
    assert len(df) > 0
# ----------------------
