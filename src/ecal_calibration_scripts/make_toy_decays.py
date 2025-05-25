'''
Script with code meant to create JSON files with
toy decays, needed for tests
'''
import os
from importlib.resources import files

import uproot
import pandas as pnd

# ------------------------------------
class Data:
    '''
    data class
    '''
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
            'Kp_0_PT_TRUE'  : 'kp_pt' ,
            'Kp_0_eta_TRUE' : 'kp_eta',
            'Kp_0_phi_TRUE' : 'kp_phi',
            # ----------
            'ep_0_PT_TRUE'  : 'ep_pt' ,
            'ep_0_eta_TRUE' : 'ep_eta',
            'ep_0_phi_TRUE' : 'ep_phi',
            # ----------
            'em_0_PT_TRUE'  : 'em_pt' ,
            'em_0_eta_TRUE' : 'em_eta',
            'em_0_phi_TRUE' : 'em_phi',
            }
# ------------------------------------
def _get_df() -> pnd.DataFrame:
    root_path = f'{Data.ana_dir}/Rapidsim/{Data.ntup_ver}/bpkpee/13TeV/bpkpee_tree.root'
    ifile = uproot.open(root_path)
    tree  = ifile['DecayTree']
    df    = tree.arrays(Data.l_branch, library='pd', entry_stop=10_000)
    df    = df.rename(columns=Data.d_name)

    return df
# ------------------------------------
def main():
    '''
    Start here
    '''
    df = _get_df()

    out_dir = files('ecal_calibration_data').joinpath('tests/data')
    os.makedirs(out_dir, exist_ok=True)

    out_path = f'{out_dir}/toy_decays.json'
    df.to_json(out_path, indent=2)
# ------------------------------------
if __name__ == '__main__':
    main()
