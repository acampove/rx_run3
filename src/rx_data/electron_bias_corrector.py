'''
Module with ElectronBiasCorrector class
'''
import pandas as pnd

# ---------------------------------
class ElectronBiasCorrector:
    '''
    Class meant to correct electron kinematics
    '''
    # ---------------------------------
    def __init__(self):
        pass
    # ---------------------------------
    def correct(self, row : pnd.Series, name : str) -> pnd.Series:
        '''
        Corrects kinematics and returns row
        row  : Pandas dataframe row
        name : Particle name, e.g. L1
        '''
        if not getattr(row, f'{name}_HASBREMADDED'):
            return row

        return row
# ---------------------------------
