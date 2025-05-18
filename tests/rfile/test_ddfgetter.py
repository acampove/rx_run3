'''
Script used to test the DDataFrameGetter class
'''

from importlib.resources   import files

#from dmu.rfile.ddfgetter   import DDataFrameGetter
from dmu.testing.utilities import build_friend_structure

# ------------------------------
def test_simple():
    '''
    Simplest test for loading trees and friends into a DaskDataFrame
    '''
    file_name = 'friends.yaml'
    build_friend_structure(file_name=file_name)
    cfg_path  = files('dmu_data').joinpath(f'rfile/{file_name}')

    ddfg = DDataFrameGetter(config_path=cfg_path)
    ddf  = ddfg.get_dataframe()
# ------------------------------
