'''
Scrip used to plot effect of corrections due to biases
in electrons
'''
import os
import re
import glob
import argparse

import mplhep
import pandas                as pnd
import dmu.generic.utilities as gut
import matplotlib.pyplot     as plt

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_plots:mass_bias_correction')
# --------------------------------------
class Data:
    '''
    Class used to hold attributes
    '''
    sample  : str

    ana_dir = os.environ['ANADIR']
    kind    = 'brem_track_2'
    trigger = 'Hlt2RD_BuToKpEE_MVA'

    regex   = f'{ana_dir}\/\w+\/\w+\/\w+\/\w+\/\w+\/\w+\/(\d)_(\d)\/(\w+)\/([\w,\(,\)]+.json)'
    d_naming= {
            'bmass_Original'               : 'Out of the Box',
            'bmass_Corrected'              : 'Corrected',
            'bmass_Corrected_and_L0(nPVs)' : '+Run1/2 Emulation',
            }

    plt.style.use(mplhep.style.LHCb2)
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
#-------------------------------------
def _reorder_blocks(df : pnd.DataFrame) -> pnd.DataFrame:
    custom_order = ['0', '4', '3', '1', '2', '1', '5', '6', '7', '8']
    df['block']  = df['block'].astype(str)

    order_map = {val: i for i, val in enumerate(custom_order)}
    df['order'] = df['block'].map(order_map)

    df= df.sort_values('order')

    return df
# --------------------------------------
def _pad_blocks(df : pnd.DataFrame) -> pnd.DataFrame:
    if Data.sample.startswith('DATA'):
        return df

    df_b2 = df[df['block'] == '2']
    df_b0 = df_b2.copy()
    df_b4 = df_b2.copy()
    df_b3 = df_b2.copy()

    df_b0['block'] = '0'
    df_b4['block'] = '4'
    df_b3['block'] = '3'

    df = pnd.concat([df_b0, df_b4, df_b3, df], axis=0, ignore_index=True)

    return df
# --------------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to go from JSON files with statistics regarding the Brem track correction to summary plots')
    parser.add_argument('-s', '--sample' , type=str, help='Sample', choices=['dat', 'sim'], required=True)
    args = parser.parse_args()

    Data.sample = 'DATAp' if args.sample == 'dat' else 'Bu_JpsiK_ee_eq_DPC'
# --------------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    l_path = _get_paths()
    l_df   = [ _path_to_df(path) for path in l_path ]

    df         = pnd.concat(l_df, axis=0, ignore_index=True)
    df         = _pad_blocks(df)
    df         = _reorder_blocks(df)
    df['kind'] = df['kind'].replace(Data.d_naming)

    for mva, df_mva in df.groupby('mva'):
        for brem, df_brem in df_mva.groupby('brem'):
            ax = None
            for kind, df_kind in df_brem.groupby('kind'):
                ax = df_kind.plot(x='block', y='fwhm', label=kind, ax=ax, figsize=[15,10])

            plt.title(f'MVA: {mva}; Brem: {brem}')

            plot_path = f'{Data.ana_dir}/plots/comparison_{Data.kind}/{Data.kind}/{Data.sample}/mva_{mva}_brem_{brem}.png'
            plt.ylim(000, 500)
            plt.xlabel('Block')
            plt.ylabel('FWHM [MeV]')
            plt.grid()
            plt.savefig(plot_path)
            plt.close()
# --------------------------------------
if __name__ == '__main__':
    main()
