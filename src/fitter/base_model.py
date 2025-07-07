'''
Module containing BaseModel class
'''

class BaseModel:
    '''
    Model base class, meant to

    - Contain common functionalities for SimModel and DataModel
    - Behave as a dependency sink, avoiding circular imports
    '''
    def __init__(self):
        '''

        '''
