'''
Script used to calculate brem fractions
'''
import os
import mplhep
import argparse
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                   import RDataFrame
from dmu.logging.log_store  import LogStore
from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter

log=LogStore.add_logger('rx_q2:calculate_brem')
# --------------------------------------
class Data:
    '''
    Data class
    '''
    l_branch = [
            'L1_HASBREMADDED',
            'L2_HASBREMADDED',
            'L1_HASBREMADDED_brem_track_2',
            'L2_HASBREMADDED_brem_track_2',
            'B_const_mass_M',
                ]

    trigger   = 'Hlt2RD_BuToKpEE_MVA'
    q2bin     = 'jpsi'
    json_path = 'data.json'
    plt.style.use(mplhep.style.LHCb2)
# --------------------------------------
def _get_rdf(sample : str) -> RDataFrame:
    gtr = RDFGetter(sample=sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()

    d_sel        = sel.selection(trigger=Data.trigger, q2bin=Data.q2bin, process=sample)
    d_sel['bdt'] = 'mva_cmb > 0.8 && mva_prc > 0.8'
    d_sel['mass']= 'B_const_mass_M > 5180'

    for cut_name, cut_expr in d_sel.items():
        rdf = rdf.Filter(cut_expr, cut_name)

    return rdf
# --------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script needed to calculate brem fractions from data and MC')
    _      = parser.parse_args()
# --------------------------------------
def _attach_brem(df : pnd.DataFrame) -> pnd.DataFrame:
    df['nbrem_org'] = df['L1_HASBREMADDED']              + df['L2_HASBREMADDED']
    df['nbrem_cor'] = df['L1_HASBREMADDED_brem_track_2'] + df['L2_HASBREMADDED_brem_track_2']

    return df
# --------------------------------------
def _plot_brem(df : pnd.DataFrame, kind : str):
    df_dt = df[df['sample']==              'DATA*']
    df_mc = df[df['sample']== 'Bu_JpsiK_ee_eq_DPC']

    _, ax = plt.subplots(figsize=[10,8])
    df_dt[kind].hist(bins=3, range=[0, 3],       alpha=0.5, color='gray', density=True, label='Data')
    df_mc[kind].hist(bins=3, range=[0, 3], histtype='step', color='blue', density=True, label='MC')
    ax.legend()
    plt.xlabel('Brem category')
    plt.ylabel('Fraction')
    plt.ylim(0, 1)
    plt.title(r'MVA${}_{cmb}>0.8$; MVA${}_{prc}>0.8$; $M_{DTF}(B^+)>5180$')
    plt.savefig(f'{kind}.png')
    plt.close()
# --------------------------------------
def _plot_mass(df : pnd.DataFrame, kind : str):
    df_dt = df[df['sample']==              'DATA*']
    df_mc = df[df['sample']== 'Bu_JpsiK_ee_eq_DPC']

    _, ax = plt.subplots(figsize=[10,8])
    df_dt[kind].hist(bins=60, range=[5000, 5800],       alpha=0.5, color='gray', density=True, label='Data')
    df_mc[kind].hist(bins=60, range=[5000, 5800], histtype='step', color='blue', density=True, label='MC')
    ax.legend()
    plt.xlabel('M${}_{DTF}(B^+)$[MeV]')
    plt.ylabel('Normalized')
    plt.title(r'MVA${}_{cmb}>0.8$; MVA${}_{prc}>0.8$; $M_{DTF}(B^+)>5180$')
    plt.savefig(f'{kind}.png')
    plt.close()
# --------------------------------------
def _get_brem_fractions(rdf : RDataFrame) -> pnd.DataFrame:
    data                  = rdf.AsNumpy(Data.l_branch)
    df                    = pnd.DataFrame(data)
    df['L1_HASBREMADDED'] = df['L1_HASBREMADDED'].astype(int)
    df['L2_HASBREMADDED'] = df['L2_HASBREMADDED'].astype(int)

    df = _attach_brem(df)
    df = df[[col for col in df.columns if col.startswith('nbrem') or col.startswith('B_const') ]]

    return df
# --------------------------------------
def _get_df() -> pnd.DataFrame:
    if os.path.isfile(Data.json_path):
        log.info(f'Dataframe found, loading: {Data.json_path}')
        df = pnd.read_json(Data.json_path)
        return df

    l_df = []
    for sample in ['DATA*', 'Bu_JpsiK_ee_eq_DPC']:
        rdf          = _get_rdf(sample=sample)
        df           = _get_brem_fractions(rdf=rdf)
        df['sample'] = sample

        l_df.append(df)

    df = pnd.concat(l_df, axis=0, ignore_index=True)
    df.to_json(Data.json_path, indent=2)

    return df
# --------------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    df = _get_df()

    _plot_brem(df, 'nbrem_org')
    _plot_brem(df, 'nbrem_cor')

    _plot_mass(df, 'B_const_mass_M')
# --------------------------------------
if __name__ == '__main__':
    main()
