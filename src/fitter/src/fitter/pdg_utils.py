'''
Module with functions intended to interface with the PDG API
'''
import pdg
from pdg.particle import PdgParticle

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:pdg_utils')

# TODO: We should not connect to the PDF to get a few branching fractions
# Put the BF in a YAML file and load it from there
#-------------------------------------------------------
def get_bf(decay : str) -> float:
    '''
    Returns branching fraction for a given decay
    '''
    # See: https://github.com/particledatagroup/api/issues/28
    if decay == 'B0 --> J/psi(1S) K*(892)0':
        return 1.27e-3

    api    = pdg.connect()
    mother = decay.split('-->')[0].replace(' ', '')
    part   = api.get_particle_by_name(mother)
    if not isinstance(part, PdgParticle):
        raise TypeError('Expected list of PDGParticles')

    l_bfrc = part.exclusive_branching_fractions()

    for bf in l_bfrc: 
        if bf.is_limit:
            continue

        if bf.description == decay:
            return bf.value

    log.error(f'Cannot find BF for decay: {decay}')
    raise ValueError('Make sure your pdg>=0.1.2')
#-------------------------------------------------------
