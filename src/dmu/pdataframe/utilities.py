'''
Module containing utilities for pandas dataframes
'''
import os
import pandas as pnd

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('dmu:pdataframe:utilities')

# -------------------------------------
def df_to_tex(df         : pnd.DataFrame,
              path       : str,
              hide_index : bool         = True,
              d_format   : dict[str,str]= None,
              **kwargs   : str       ) -> None:
    '''
    Saves pandas dataframe to latex

    Parameters
    -------------
    df              : Dataframe with data
    path     (str)  : Path to latex file
    hide_index      : If true (default), index of dataframe won't appear in table
    d_format (dict) : Dictionary specifying the formattinng of the table, e.g. `{'col1': '{}', 'col2': '{:.3f}', 'col3' : '{:.3f}'}`
    kwargs          : Arguments needed in `to_latex`
    '''

    if path is not None:
        dir_name = os.path.dirname(path)
        os.makedirs(dir_name, exist_ok=True)

    st = df.style
    if hide_index:
        st=st.hide(axis='index')

    if d_format is not None:
        st=st.format(formatter=d_format)

    log.info(f'Saving to: {path}')
    buf = st.to_latex(buf=path, hrules=True, **kwargs)

    return buf
# -------------------------------------
