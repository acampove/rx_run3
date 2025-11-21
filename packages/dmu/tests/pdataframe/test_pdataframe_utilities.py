'''
Module will hold test for pandas dataframes utilities
'''
import os
import pytest
import numpy
import pandas                   as pnd
import dmu.pdataframe.utilities as put

from colorama              import Fore
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test:pdataframe:utilities')
# --------------------------------------
class Data:
    '''
    Data class
    '''
    user    = os.environ['USER']
    out_dir = f'/tmp/{user}/tests/dmu/pdataframe/utilities'
# --------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('dmu:pdataframe:utilities', 10)
# --------------------------------------
def _get_df() -> pnd.DataFrame:
    d_data = {}
    d_data['a'] = [1,2,3]
    d_data['b'] = [4,5,6]
    d_data['c'] = [7,8,9]

    return pnd.DataFrame(d_data)
# --------------------------------------
def test_df_to_tex_simple():
    '''
    Saving dataframe to latex table
    '''
    df = _get_df()
    put.df_to_tex(df, f'{Data.out_dir}/df_to_tex/simple.tex')
# --------------------------------------
def test_df_to_markdown():
    '''
    Saving dataframe to markdown 
    '''
    df = _get_df()
    put.to_markdown(df, f'{Data.out_dir}/df_to_markdown/simple.md')
# --------------------------------------
def test_df_to_tex_format():
    '''
    Saving dataframe to latex table with formatting for columns
    '''
    d_format = {
            'a' : '{:.0f}',
            'b' : '{:.3f}'}

    df = _get_df()
    put.df_to_tex(
            df,
            f'{Data.out_dir}/df_to_tex/format.tex',
            d_format = d_format)
# --------------------------------------
def test_df_to_tex_caption():
    '''
    Saving dataframe to latex table with caption
    '''
    df = _get_df()
    put.df_to_tex(df,
                  f'{Data.out_dir}/df_to_tex/caption.tex',
                  caption = 'Some caption')
# --------------------------------------
def test_column_format():
    '''
    Test alignment of columns
    '''
    df = _get_df()
    put.df_to_tex(df, f'{Data.out_dir}/df_to_tex/column_format.tex', column_format='lrr')
# --------------------------------------
@pytest.mark.parametrize('yml_path',[
                              '/tmp/tests/dmu/pdataframe/utilities/to_yaml/file.yaml',
                              'test_file.yaml'])
def test_to_from_yaml(yml_path : str):
    '''
    Tests saving dataframe to YAML
    '''

    df_1 = _get_df()
    put.to_yaml(df_1, yml_path)
    df_2 = put.from_yaml(yml_path)

    assert df_1.equals(df_2)

    os.unlink(yml_path)
# --------------------------------------
@pytest.mark.parametrize('nan_frac', [0.00, 0.01, 0.05])
def test_dropna(nan_frac : float):
    '''
    Tests wrapper to dropna from Pandas
    '''
    arr  = numpy.random.rand(10_000)
    mask = numpy.random.rand(10_000) < nan_frac
    arr[mask] = numpy.nan
    df   = pnd.DataFrame({'a' : arr})

    if nan_frac < 0.02:
        df   = put.dropna(df)

        assert not df.isna().any().any()

        return

    with pytest.raises(ValueError):
        df   = put.dropna(df)
# --------------------------------------
def test_colorize():
    '''
    Tests function that adds colors to dataframes
    '''
    df     = _get_df()
    colors = {1 : Fore.RED, 2 : Fore.BLUE}
    df     = df.apply(put.colorize_row, args=(colors,), axis=1)

    log.info('')
    print(df.to_markdown())
