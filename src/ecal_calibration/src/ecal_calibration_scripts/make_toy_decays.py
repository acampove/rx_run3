'''
Script with code meant to create JSON files with
toy decays, needed for tests
'''
import os
import argparse
from importlib.resources import files

import numpy
import uproot
import pandas as pnd

from vector import MomentumObject3D as v3d
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('ecal_calibration:make_toy_decays')
# ------------------------------------
class Data:
    '''
    data class
    '''
    nentries = 10_000
    ntup_ver = 'v2'
    ana_dir  = os.environ['ANADIR']
    l_branch = [
        'Kp_0_PT_TRUE',
        'ep_0_PT_TRUE',
        'em_0_PT_TRUE',
        'Kp_0_eta_TRUE',
        'ep_0_eta_TRUE',
        'em_0_eta_TRUE',
        'Kp_0_phi_TRUE',
        'ep_0_phi_TRUE',
        'em_0_phi_TRUE',
        ]

    d_name= {
            'Kp_0_PT_TRUE'  :  'H_PT' ,
            'Kp_0_eta_TRUE' :  'H_ETA',
            'Kp_0_phi_TRUE' :  'H_PHI',
            # ----------
            'ep_0_PT_TRUE'  : 'L1_PT' ,
            'ep_0_eta_TRUE' : 'L1_ETA',
            'ep_0_phi_TRUE' : 'L1_PHI',
            # ----------
            'em_0_PT_TRUE'  : 'L2_PT' ,
            'em_0_eta_TRUE' : 'L2_ETA',
            'em_0_phi_TRUE' : 'L2_PHI'}
# ------------------------------------
def _get_df() -> pnd.DataFrame:
    root_path = f'{Data.ana_dir}/Rapidsim/{Data.ntup_ver}/bpkpee/13TeV/bpkpee_tree.root'
    log.info(f'Reading datafrom: {root_path}')
    ifile = uproot.open(root_path)
    tree  = ifile['DecayTree']
    df    = tree.arrays(Data.l_branch, library='pd', entry_stop=Data.nentries)
    df    = df.rename(columns=Data.d_name)

    return df
# ------------------------------------
def _add_b_vtx(row : pnd.Series) -> pnd.Series:
    ep = v3d(pt=row.L1_PT, eta=row.L1_ETA, phi=row.L1_PHI)
    em = v3d(pt=row.L2_PT, eta=row.L2_ETA, phi=row.L2_PHI)
    kp = v3d(pt=row.H_PT , eta=row.H_ETA , phi=row.H_PHI )

    bp = ep + em + kp
    pos= numpy.random.exponential(scale=10) * bp / bp.mag
    sr = pnd.Series(
            {'B_END_VX' : pos.px,
             'B_END_VY' : pos.py,
             'B_END_VZ' : pos.pz,
             'B_BPVX'   : 0,
             'B_BPVY'   : 0,
             'B_BPVZ'   : 0})

    return sr
# ------------------------------------
def _reformat_df(df : pnd.DataFrame) -> pnd.DataFrame:
    df_vtx = df.apply(_add_b_vtx, axis=1)

    df     = df.join(df_vtx)
    df     = _add_lepton_columns(df=df, lepton='L1')
    df     = _add_lepton_columns(df=df, lepton='L2')
    df     = _add_event_columns(df=df)

    return df
# ------------------------------------
def _add_event_columns(df : pnd.DataFrame) -> pnd.DataFrame:
    size        = len(df)
    df['nPVs' ] = numpy.random.poisson(lam=3, size=size)
    df['block'] = numpy.random.randint(1, 9,  size=size)

    return df
# ------------------------------------
def _add_lepton_columns(df : pnd.DataFrame, lepton : str) -> pnd.DataFrame:
    size   = len(df)

    df[f'{lepton}_BREMHYPOAREA'] = numpy.random.choice([0, 1, 2], size=size)
    df[f'{lepton}_brem'        ] = numpy.random.choice([0, 1   ], size=size, p=[0.2, 0.8])
    df[f'{lepton}_BREMHYPOCOL' ] = numpy.random.randint(1, 60, size=size)
    df[f'{lepton}_BREMHYPOROW' ] = numpy.random.randint(1, 60, size=size)
    df[f'{lepton}_BREMTRACKBASEDENERGY' ] = numpy.random.exponential(scale=30_000, size=size)

    return df
# ------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to produce JSON file with toy decays needed for running tests')
    parser.add_argument('-n', '--nentries', type=int, help='Number of entries to process', default=Data.nentries)
    args = parser.parse_args()

    Data.nentries = args.nentries
# ------------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    df = _get_df()
    df = _reformat_df(df=df)

    out_dir = files('ecal_calibration_data').joinpath('tests/data')
    os.makedirs(out_dir, exist_ok=True)

    out_path = f'{out_dir}/toy_decays.parquet'
    log.info(f'Sending output to: {out_path}')
    df.to_parquet(out_path, compression='snappy')
# ------------------------------------
if __name__ == '__main__':
    main()
