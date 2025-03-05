'''
Module with code needed to retrieve X, Y position from ECAL cell ID 
'''

# ------------------------------------------------------
def from_id_to_xy(row : int, col : int) -> tuple[float,float]:
    '''
    Function taking row and column in ECAL
    returning X,Y coordinates
    '''
    return row, col
# ------------------------------------------------------
