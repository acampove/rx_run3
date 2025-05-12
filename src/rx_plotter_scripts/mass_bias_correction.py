'''
Scrip used to plot effect of corrections due to biases
in electrons
'''
import os
import re
import glob
import pandas                as pnd
import dmu.generic.utilities as gut

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_plots:mass_bias_correction')
# --------------------------------------
class Data:
    '''
    Class used to hold attributes
    '''
    ana_dir = os.environ['ANADIR']
    kind    = 'brem_track_2'
    sample  = 'DATAp'
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    regex   = f'{ana_dir}\/\w+\/\w+\/\w+\/\w+\/\w+\/\w+\/(\d)_(\d)\/(\w+)\/([\w,\(,\)]+.json)'
    d_naming= {
            'bmass_Original'               : 'Out of the Box',
            'bmass_Corrected'              : 'Corrected',
            'bmass_Corrected_and_L0(nPVs)' : '+Run1/2 Emulation',
            }
# --------------------------------------
def _get_paths() -> list[str]:
    path_wc = f'{Data.ana_dir}/plots/comparison_{Data.kind}/{Data.kind}/{Data.sample}/{Data.trigger}/jpsi/*/*/*.json'
    l_path  = glob.glob(path_wc)

    npath = len(l_path)
    if npath == 0:
        raise ValueError(f'No path found in {path_wc}')

    return l_path
# --------------------------------------
def _info_from_path(path : str) -> tuple[str,str,str,str]:
    mtch = re.match(Data.regex, path)
    if not mtch:
        raise ValueError(f'Cannot extract information from {path}')

    l_match = mtch.groups()

    try:
        brem, block, mva, kind = l_match

        kind = kind.replace('.json', '').replace('fwhm_', '')
        mva  = mva.replace('drop_mva', '0').replace('with_mva', '1')

        return brem, block, mva, kind
    except ValueError as exc:
        log.warning(f'Matched: {l_match}')
        raise ValueError from exc
# --------------------------------------
def _path_to_df(path : str) -> pnd.DataFrame:
    brem, block, mva, kind = _info_from_path(path)
    data = gut.load_json(path)

    data['brem' ] = brem
    data['block'] = block
    data['mva'  ] = mva
    data['kind' ] = kind

    return pnd.DataFrame([data])
# --------------------------------------
def main():
    '''
    Start here
    '''
    l_path = _get_paths()
    l_df   = [ _path_to_df(path) for path in l_path ]

    df         = pnd.concat(l_df, axis=0, ignore_index=True)
    df['kind'] = df['kind'].replace(Data.d_naming)

    pnd.set_option('display.max_rows', None)
    print(df)

# --------------------------------------
if __name__ == '__main__':
    main()
