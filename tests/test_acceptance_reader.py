'''
Modules used to test AcceptanceReader class
'''

import pytest
from dmu.logging.log_store             import LogStore
from rx_efficiencies.acceptance_reader import AcceptanceReader
from rx_efficiencies.decay_names       import DecayNames

log=LogStore.add_logger('rx_efficiencies:test_acceptance_reader')
#------------------------------------
@pytest.mark.parametrize('year'   , ['2018', '2024'])
@pytest.mark.parametrize('process', DecayNames.get_decays())
def test_simple(year : str, process : str):
    '''
    Test reading acceptances from JSON files for multiple processes and years
    '''
    obj=AcceptanceReader(year=year, proc=process)
    acc=obj.read()

    log.info(f'{year:<20}{process:<20}{acc:10.3f}')

    assert 0.1 < acc < 0.2
#------------------------------------
