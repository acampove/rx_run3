'''
Module with code needed to retrieve X, Y position from ECAL cell ID
'''

from importlib.resources import files
import pandas as pnd

# --------------------------------
def _cast_column(column, ctype) -> pnd.Series:
    column = pnd.to_numeric(column, errors='coerce')
    column = column.fillna(-100_000)
    column = column.astype(ctype)

    return column
# --------------------------------
def get_data() -> pnd.DataFrame:
    '''
    Returns pandas dataframe with x,y and row and column values
    '''
    data_path = files('rx_data_data').joinpath('brem_correction/coordinates.csv')
    df      = pnd.read_csv(data_path)
    df['x'] = _cast_column(df.x, float)
    df['y'] = _cast_column(df.y, float)
    df['r'] = _cast_column(df.r, int)
    df['c'] = _cast_column(df.c, int)

    df = df[df.x > - 10_000]
    df = df[df.y > - 10_000]

    return df
# ------------------------------------------------------
def from_id_to_xy(row : int, col : int) -> tuple[float,float]:
    '''
    Function taking row and column in ECAL
    returning X,Y coordinates
    '''
    df = get_data()
    df = df[df.r==row]
    df = df[df.c==col]

    return df
# ------------------------------------------------------
