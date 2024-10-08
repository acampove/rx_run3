'''
Module containing the Function class
'''

class Function:
    '''
    Class meant to represent a 1D function created from (x, y) coordinates
    '''
    def __init__(self, x : list, y : list):
        '''
        x (list) : List with x coordinates
        y (list) : List with y coordinates
        '''

        if len(x) != len(y):
            raise ValueError('X and Y coordinates have different lengths')

        self._l_x = x
        self._l_y = y
