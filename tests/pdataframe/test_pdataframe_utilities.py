'''
Module will hold test for pandas dataframes utilities
'''
import pandas as pnd

import dmu.pdataframe.utilities as put
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test:pdataframe:utilities')

# --------------------------------------
class Data:
    '''
    Data class
    '''
    out_dir = '/tmp/tests/dmu/pdataframe/utilities'
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
def test_to_from_yaml():
    '''
    Tests saving dataframe to YAML
    '''
    yml_path = '/tmp/tests/dmu/pdataframe/utilities/to_yaml/file.yaml'

    df_1 = _get_df()
    put.to_yaml(df_1, yml_path)
    df_2 = put.from_yaml(yml_path)

    assert df_1.equals(df_2)
# --------------------------------------
