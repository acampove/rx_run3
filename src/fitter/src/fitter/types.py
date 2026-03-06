'''
Module holding enums needed for fitting models
'''

from enum import StrEnum

# ------------------------------
class CCbarWeight(StrEnum):
    '''
    Weights needed to correct ccbar samples
    in order to build cocktails

    Attributes
    
    dec: Decay weights, needed to correct branching fractions within cocktail
    sam: Sample weights, needed to correct for hadronization fraction, etc between Bs, Bd and Bp decays 
    '''
    dec = 'dec'
    sam = 'sam' 
# ------------------------------
