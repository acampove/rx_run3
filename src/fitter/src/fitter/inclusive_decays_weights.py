'''
Module with Reader class
'''
# pylint: disable=too-many-instance-attributes, invalid-name

import pandas as pnd
from fitter.pchain         import PChain
from dmu.logging.log_store import LogStore
from fitter.decay_reader   import KLLDecayReader, KPiLLDecayReader

log = LogStore.add_logger('rx_fitter:inclusive_decays_weights')
#---------------------------
def _get_chain(name, row : pnd.Series) -> PChain:
    '''
    Will return an instance of a PChain for a given dataframe row
    '''
    v1 = row[f'{name}_TRUEID']
    v2 = row[f'{name}_MC_MOTHER_ID']
    v3 = row[f'{name}_MC_GD_MOTHER_ID']
    v4 = row[f'{name}_MC_GD_GD_MOTHER_ID']

    chain = PChain(v1, v2, v3, v4)

    return chain
#---------------------------
def _get_reader(project : str, row : pnd.Series) -> KLLDecayReader | KPiLLDecayReader:
    '''
    This method will return the BR weights

    Parameters
    -------------------
    row    : Row in pandas dataframe holding PDG id information for the three particles.
    project: rk, rkst, etc, used to pick particles

    Returns
    -------------------
    Reader to extract candidate weigth
    '''
    l1 = _get_chain('L1', row)
    l2 = _get_chain('L2', row)

    if   project == 'rk':
        kp = _get_chain( 'H', row)
        obj= KLLDecayReader(l1=l1, l2=l2, kp=kp)
    elif project == 'rkst':
        kp = _get_chain('H1', row)
        pi = _get_chain('H2', row)
        obj= KPiLLDecayReader(l1=l1, l2=l2, kp=kp, pi=pi)
    else:
        raise NotImplementedError(f'Invalid project: {project}')

    return obj
#---------------------------
def read_weight(row : pnd.Series, project : str) -> float:
    '''
    This method will return the BR weights

    Parameters
    -------------------
    row    : Row in pandas dataframe holding PDG id information for the three particles.
    project: rk, rkst, etc, used to pick particles

    Returns
    -------------------
    Weight for candidate associated to input row
    '''
    rdr = _get_reader(project=project, row=row)
    wgt = rdr.get_weight()

    return wgt
#---------------------------
