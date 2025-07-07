'''
This module contains BaseFitter
'''

class BaseFitter:
    '''
    Fitting base class, meant to

    - Provide basic functionality to fiters for data and simulation
    - Behave as a dependency sink, avoiding circular imports
    '''
    def __init__(self):
        '''
        '''
