'''
Modules used to test AcceptanceReader class
'''

import pytest
from dmu              import LogStore
from rx_common        import Sample
from rx_efficiencies  import AcceptanceReader

log=LogStore.add_logger('rx_efficiencies:test_acceptance_reader')
#------------------------------------
@pytest.mark.parametrize('year'   , ['2018', '2024'])
@pytest.mark.parametrize('sample' , Sample.get_mc_samples())
def test_simple(year : str, sample : Sample):
    '''
    Test reading acceptances from JSON files for multiple processes and years
    '''
    obj=AcceptanceReader(year=year, sample=sample)
    acc=obj.read()

    log.info(f'{year:<20}{sample:<20}{acc:10.3f}')

    assert 0.1 < acc < 0.2
#------------------------------------
