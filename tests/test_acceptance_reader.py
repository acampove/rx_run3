'''
Modules used to test AcceptanceReader class
'''

import pytest
from dmu.logging.log_store             import LogStore
from rx_efficiencies.acceptance_reader import AcceptanceReader

log=LogStore.add_logger('rx_efficiencies:test_acceptance_reader')
#------------------------------------
class Data:
    '''
    Data class
    '''
    l_process = ['bdkskpiee', 'bpk1kpipiee', 'bpk2kpipiee', 'bpkskpiee', 'bsphiee']
#------------------------------------
@pytest.mark.parametrize('year'   , ['2018', '2024'])
@pytest.mark.parametrize('process', Data.l_process)
def test_simple(year : str, process : str):
    '''
    Test reading acceptances from JSON files for multiple processes and years
    '''
    obj=AcceptanceReader(year=year, proc=process)
    acc=obj.read()

    log.info(f'{year:<20}{process:<20}{acc:10.3f}')

    assert 0.1 < acc < 0.2
#------------------------------------
