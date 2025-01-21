'''
Module will hold test for pandas dataframes utilities 
'''
import pandas as pnd

import dmu.pdataframe.utilities as put
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:test:pdataframe:utilities')

# --------------------------------------
def _get_df() -> pnd.DataFrame:
    d_data = {}
    d_data['a'] = [1,2,3]
    d_data['b'] = [4,5,6]

    return pnd.DataFrame(d_data)
# --------------------------------------
def test_df_to_tex():
    '''
    Saving dataframe to latex table
    '''
    df = _get_df()
    put.df_to_tex(df, '/tmp/dmu/tests/pdataframe/utilities/df_to_tex/simple.tex')
