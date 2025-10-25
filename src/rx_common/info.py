'''
This script contains functions needed to get information on samples
'''

from dmu.generic import utilities  as gut

_triggers = gut.load_data(package='rx_common_data', fpath='triggers.yaml')

# ---------------------------------
def is_mc(sample : str) -> bool:
    '''
    Given a sample name, it will check if it is MC or data
    '''

    if sample.startswith('DATA'):
        return False

    return True
# ---------------------------------
def channel_from_trigger(trigger : str) -> str:
    '''
    Parameters
    ----------------
    trigger: Hlt2 trigger name, e.g. HLT2_BuKp...

    Returns
    ----------------
    Channel, i.e. EE, MM, EM
    '''
    for project in _triggers:
        for channel in _triggers[project]:
            if trigger not in _triggers[project][channel]:
                continue

            return channel

    raise ValueError(f'Trigger {trigger} not found')
# ---------------------------------
def is_ee(trigger : str) -> bool:
    '''
    Given Hlt2 trigger name, it will tell if it belongs to electron sample
    '''

    return channel_from_trigger(trigger) == 'EE'
# ---------------------------------
def is_mm(trigger : str) -> bool:
    '''
    Given Hlt2 trigger name, it will tell if it belongs to muon sample
    '''

    return channel_from_trigger(trigger) == 'MM'
# ---------------------------------
def is_em(trigger : str) -> bool:
    '''
    Given Hlt2 trigger name, it will tell if it belongs to electron-muon sample
    '''

    return channel_from_trigger(trigger) == 'MM'
# ---------------------------------
def project_from_trigger(trigger : str, lower_case : bool) -> str:
    '''
    Parameters
    -------------------
    trigger  : HLT2 trigger
    lowe_case: If True will return rk, rkst, etc

    Returns
    -------------------
    Project, e.g RK, RKst
    '''
    for project in _triggers:
        for channel in _triggers[project]:
            if trigger not in _triggers[project][channel]:
                continue

            if lower_case:
                project = project.lower()

            return project

    raise ValueError(f'Trigger {trigger} not found')
# ---------------------------------
def is_reso(q2bin : str) -> bool:
    '''
    Takes q2bin name, returns true if it has associated
    charmonium component. If not, returns false.
    '''
    reso = ['jpsi', 'psi2']
    rare = ['low', 'cen_low', 'central', 'cen_high', 'high']

    if q2bin in reso:
        return True

    if q2bin in rare:
        return False

    raise ValueError(f'Invalid q2bin: {q2bin}')
