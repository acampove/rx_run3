'''
This module contains functions meant to be shared by code in rx_efficiencies
'''
from rx_common import Project, Component

# ---------------------------------------
def is_acceptance_defined(sample : Component, project : Project) -> bool:
    '''
    Parameters
    -------------
    sample : MC sample, e.g. bpkpee
    project: E.g. rk or rkst

    Returns
    -------------
    True or false. False if for this project this samples' acceptance does not make sense 
    '''
    if project not in {Project.rk, Project.rkst}:
        raise ValueError(f'Invalid project: {project}')

    if project == Project.rk:
        return True 

    # These samples do not have a pion
    # and cannot be used to calculate geometric
    # acceptance under rkst hypothesis
    rk_only_samples = {
        Component.bsphiee,
        Component.bpkstkpiee, # This is a neutral pion
        Component.bpkpmm    , Component.bpkpee, 
        Component.bpkpjpsiee, Component.bpkpjpsimm,
        Component.bpkppsi2ee, Component.bpkppsi2mm,
    }

    if sample in rk_only_samples:
        return False 

    return True 
# ---------------------------------------

