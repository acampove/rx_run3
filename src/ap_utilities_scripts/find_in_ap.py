'''
This script is meant to:

- Search for all the lines in:

$ANADIR/bkk_checker/block_*/info.yaml

- Check if ntuples corresponding to each line exist in BKK.
- Build a new info.yaml for missing samples
'''
import apd

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ap_utilities:find_in_ap')
# ----------------------
def _get_event_types() -> dict[int, str]:
    '''
    Returns
    -------------
    Dictionary with:

    Key  : EventType
    Value: Line that would go in info.yaml
    '''
    dset = apd.get_analysis_data(working_group='RD', analysis='rd_ap_2024')

# ----------------------
def main():
    '''
    Entry point
    '''
    d_evt_type = _get_event_types()
    d_missing  = { evt_type : path for evt_type, path in d_evt_type.items() if _found_type(evt_type) }

    with open('info.yaml', 'w') as ofile:
        for line in d_missing.values():
            ofile.write(f'{line}\n')
# ----------------------
if __name__ == '__main__':
    main()n
