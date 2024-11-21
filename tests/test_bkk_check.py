'''
Module with tests for BkkChecker class
'''
from importlib.resources import files

from ap_utilities.bookkeeping.bkk_checker import BkkChecker 

# ----------------------------------------
def test_simple():
    '''
    Will save list of samples to YAML
    '''
    samples_path = files('ap_utilities_data').joinpath('rd_samples.yaml')

    obj=BkkChecker(samples_path)
    obj.save(path='existing_samples.yaml')
# ----------------------------------------
