'''
Module meant to store types relevant for statistical analysis
'''

from enum import StrEnum

# --------------------------
class Model(StrEnum):
    '''
    Enum meant to represent fitting models
    '''
    exp    = 'exp'
    hypexp = 'hypexp'
    modexp = 'modexp'
    pol1   = 'pol1'
    pol2   = 'pol2'
    pol3   = 'pol3'
    cbr    = 'cbr'
    cbl    = 'cbl'
    suj    = 'suj'
    gauss  = 'gauss'
    dscb   = 'dscb'
    voigt  = 'voigt'
    qgauss = 'qgauss'
    cauchy = 'cauchy'
# --------------------------
class KDEModel(StrEnum):
    '''
    Enum meant to represent KDE PDFs 
    '''
    kde_fft = 'KDE1DimFFT'
# --------------------------
