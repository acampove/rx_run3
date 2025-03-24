'''
Module with utility functions
'''
# pylint: disable=too-many-return-statements

import os
import re
from dataclasses            import dataclass

import pandas as pnd
from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore

log   = LogStore.add_logger('rx_data:utilities')
# ---------------------------------
@dataclass
class Data:
    '''
    Class used to hold shared data
    '''
    # pylint: disable = invalid-name
    # Need to call var Max instead of max

    dt_rgx  = r'(data_\d{2}_.*c\d)_(Hlt2RD_.*(?:EE|MuMu|misid|cal|MVA|LL|DD))_?(\d{3}_\d{3}|[a-z0-9]{10})?\.root'
    mc_rgx  = r'mc_.*_\d{8}_(.*)_(\w+RD_.*)_(\d{3}_\d{3}|\w{10}).root'
# ---------------------------------
def info_from_path(path : str) -> tuple[str,str]:
    '''
    Will pick a path to a ROOT file
    Will return tuple with information associated to file
    This is needed to name output file and directories
    '''

    name = os.path.basename(path)
    if   name.startswith('dt_') or name.startswith('data_'):
        info = _info_from_data_path(path)
    elif name.startswith('mc_'):
        info = _info_from_mc_path(path)
    else:
        log.error(f'File name is not for data or MC: {name}')
        raise ValueError

    return info
# ---------------------------------
def _info_from_mc_path(path : str) -> tuple[str,str]:
    '''
    Will return information from path to file
    '''
    name = os.path.basename(path)
    mtch = re.match(Data.mc_rgx, name)
    if not mtch:
        raise ValueError(f'Cannot extract information from MC file:\n\n{name}\n\nUsing {Data.mc_rgx}')

    try:
        [sample, line, _] = mtch.groups()
    except ValueError as exc:
        raise ValueError(f'Expected three elements in: {mtch.groups()}') from exc

    return sample, line
# ---------------------------------
def _info_from_data_path(path : str) -> tuple[str,str]:
    '''
    Will get info from data path
    '''
    name = os.path.basename(path)
    mtch = re.match(Data.dt_rgx, name)
    if not mtch:
        raise ValueError(f'Cannot find kind in:\n\n{name}\n\nusing\n\n{Data.dt_rgx}')

    try:
        [sample, line, _] = mtch.groups()
    except ValueError as exc:
        raise ValueError(f'Expected three elements in: {mtch.groups()}') from exc

    sample = sample.replace('_turbo_', '_')
    sample = sample.replace('_full_' , '_')

    return sample, line
# ---------------------------------
def df_from_rdf(rdf : RDataFrame) -> pnd.DataFrame:
    '''
    Utility method needed to get pandas dataframe from ROOT dataframe
    '''
    rdf    = _preprocess_rdf(rdf)
    l_col  = [ name.c_str() for name in rdf.GetColumnNames() if _pick_column(name.c_str()) ]
    d_data = rdf.AsNumpy(l_col)
    df     = pnd.DataFrame(d_data)

    sr_nan = df.isna().any(axis=1)
    nnan   = sr_nan.sum()
    if nnan != 0:
        log.warning(f'Found {nnan} NaNs in Pandas dataframe')
        df_nan = df[sr_nan]
        for name in df_nan.columns:
            sr_val = df_nan[name]
            if name in ['EVENTNUMBER', 'RUNNUMBER']:
                log.info(sr_val)

            if not sr_val.isna().any():
                continue
            log.info(sr_val)

        df = df.dropna()

    return df
# ------------------------------------------
def _preprocess_rdf(rdf: RDataFrame) -> RDataFrame:
    rdf = _preprocess_lepton(rdf, 'L1')
    rdf = _preprocess_lepton(rdf, 'L2')
    rdf = _preprocess_lepton(rdf,  'H')

    return rdf
# ------------------------------------------
def _preprocess_lepton(rdf : RDataFrame, lep : str) -> None:
    # Make brem flag an int (will make things easier later)
    rdf = rdf.Redefine(f'{lep}_HASBREMADDED'        , f'int({lep}_HASBREMADDED)')
    # If there is no brem, make energy zero
    rdf = rdf.Redefine(f'{lep}_BREMHYPOENERGY'      , f'{lep}_HASBREMADDED == 1 ? {lep}_BREMHYPOENERGY : 0')
    # If track based energy is NaN, make it zero
    rdf = rdf.Redefine(f'{lep}_BREMTRACKBASEDENERGY', f'{lep}_BREMTRACKBASEDENERGY == {lep}_BREMTRACKBASEDENERGY ? {lep}_BREMTRACKBASEDENERGY : 0')

    return rdf
# ------------------------------------------
def _pick_column(name : str) -> bool:
    to_keep  = ['EVENTNUMBER', 'RUNNUMBER']

    if '_TRUE' in name:
        return False

    if name in to_keep:
        return True

    not_l1 = not name.startswith('L1')
    not_l2 = not name.startswith('L2')
    not_kp = not name.startswith('H')

    if not_l1 and not_l2 and not_kp:
        return False

    if 'BREMTRACKBASEDENERGY' in name:
        return True

    if 'HASBREMADDED' in name:
        return True

    if 'NVPHITS' in name:
        return False

    if 'CHI2' in name:
        return False

    if 'HYPOID' in name:
        return False

    if 'HYPODELTA' in name:
        return False

    if 'PT' in name:
        return True

    if 'ETA' in name:
        return True

    if 'PHI' in name:
        return True

    if 'PX' in name:
        return True

    if 'PY' in name:
        return True

    if 'PZ' in name:
        return True

    if 'BREMHYPO' in name:
        return True

    return False
# ------------------------------------------
