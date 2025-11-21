'''
Module containing tests for DataVarsAdder class
'''
import os

import pytest
from ROOT import RDataFrame

from dmu.logging.log_store   import LogStore
from post_ap.data_vars_adder import DataVarsAdder

log = LogStore.add_logger('post_ap:test_data_vars_adder')
# ---------------------------------------------
def _get_rdf() -> RDataFrame:
    cernbox   = os.environ['CERNBOX']
    file_path = f'{cernbox}/Run3/analysis_productions/for_local_tests/dt_turbo.root'
    tree_path =  'Hlt2RD_BuToKpEE_MVA/DecayTree'

    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'Cannot find: {file_path}')

    rdf = RDataFrame(tree_path, file_path)

    return rdf
# ---------------------------------------------
@pytest.mark.parametrize('itry', [1,2])
def test_simple(itry : int) -> None:
    '''
    Simplest test for adding data variables
    '''

    rdf    = _get_rdf()
    obj    = DataVarsAdder(rdf)
    rdf    = obj.get_rdf()
    l_name = obj.names + ['RUNNUMBER', 'FillNumber']

    file_path = f'/tmp/var_adder_{itry:03}.root'
    log.info(f'Saving to: {file_path}')

    rdf.Snapshot('tree', file_path, l_name)
