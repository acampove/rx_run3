'''
Script used to study effect of different cuts on q2
to get signal in high q2 bin
'''
import os
import argparse

import numpy
import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                  import RDataFrame
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_plots:high_q2cut')
# ---------------------------
class Data:
    '''
    Data class
    '''
    plt_dir : str
    sample  : str
    trigger : str

    d_samples = {
            'data_ss' : 'DATA*',
            'buhsee'  : 'Bu_JpsiX_ee_eq_JpsiInAcc',
            'bdhsee'  : 'Bd_JpsiX_ee_eq_JpsiInAcc',
            'bshsee'  : 'Bs_JpsiX_ee_eq_JpsiInAcc',
            'bukee'   : 'Bu_Kee_eq_btosllball05_DPC',
            'bukjpee' : 'Bu_JpsiK_ee_eq_DPC',
            'bukpsee' : 'Bu_psi2SK_ee_eq_DPC'}

    d_latex   = {
            'DATA*'                      : 'SS data',
            'Bu_JpsiX_ee_eq_JpsiInAcc'   : r'$B^+\to H_s c\bar{c}(\to e^+e^-)$',
            'Bd_JpsiX_ee_eq_JpsiInAcc'   : r'$B^0\to H_s c\bar{c}(\to e^+e^-)$',
            'Bs_JpsiX_ee_eq_JpsiInAcc'   : r'$B_s\to H_s c\bar{c}(\to e^+e^-)$',
            'Bu_Kee_eq_btosllball05_DPC' : r'$B^+\to K^+e^+e^-$',
            'Bu_JpsiK_ee_eq_DPC'         : r'$B^+\to K^+J/\psi(\to e^+e^-)$',
            'Bu_psi2SK_ee_eq_DPC'        : r'$B^+\to K^+\psi(2S)(\to e^+e^-)$'}

    q2bin   = 'high'
    bmass   = 'B_Mass_smr'
    plt.style.use(mplhep.style.LHCb2)
    max_q2  = 22
    min_q2  = 10
    mass_rng= [4500, 6000]

    l_q2var = ['q2_smr', 'q2_track', 'q2_dtf', 'nbrem', 'B_Mass', 'B_M', 'B_Mass_smr']
# ---------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-s', '--sample', type=str, help='MC sample', required=True, choices=list(Data.d_samples))
    args = parser.parse_args()

    sample     = args.sample
    Data.sample= Data.d_samples[sample]
# ---------------------------
def _initialize():
    ana_dir = os.environ['ANADIR']
    plt_dir = f'{ana_dir}/plots/high_q2/{Data.sample}'

    os.makedirs(plt_dir, exist_ok=True)

    if Data.sample.startswith('DATA'):
        Data.trigger = 'Hlt2RD_BuToKpEE_SameSign_MVA'
    else:
        Data.trigger = 'Hlt2RD_BuToKpEE_MVA'

    Data.plt_dir = plt_dir
# ---------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf()

    d_sel       = sel.selection(trigger=Data.trigger, q2bin=Data.q2bin, process=Data.sample)
    d_sel['q2'] = '(1)'

    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    return rdf
# ---------------------------
def _reformat_q2(df : pnd.DataFrame) -> pnd.DataFrame:
    l_col = [ col for col in df.columns if col.startswith('q2') ]

    for col in l_col:
        df[col] = df[col] / 1e6

    return df
# ---------------------------
def _plot(rdf : RDataFrame) -> None:
    data = rdf.AsNumpy(Data.l_q2var)
    df   = pnd.DataFrame(data)
    df   = _reformat_q2(df=df)

    for brem, df_brem in df.groupby('nbrem'):
        _plot_q2(brem, df_brem)

    _plot_q2('all', df)
# ---------------------------
def _plot_eff(arr_val : numpy.ndarray, color : str, ax) -> None:
    sorted_data = numpy.sort(arr_val)
    eff         = 1 - numpy.arange(1, len(arr_val) + 1) / len(arr_val)

    ax.plot(sorted_data, eff, color=color)
# ---------------------------
def _plot_q2(brem : int, df : pnd.DataFrame) -> None:
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(30, 10))

    ax1.hist(df['q2_smr'  ], bins=60, range=[0, Data.max_q2], density=True, label='$q^2$'        , alpha = 0.2, color='blue' )
    ax1.hist(df['q2_dtf'  ], bins=60, range=[0, Data.max_q2], density=True, label='$q^2_{DTF}$'  , alpha = 0.2, color='green')
    ax1.hist(df['q2_track'], bins=60, range=[0, Data.max_q2], density=True, label='$q^2_{track}$', alpha = 0.2, color='red'  )

    df_smr=df[(df.q2_smr   > 15) & (df.q2_smr   < 22)                     ]
    df_dtf=df[(df.q2_dtf   > 15) & (df.q2_dtf   < 22) & (df.q2_smr   < 22)]
    df_trk=df[(df.q2_track > 15) & (df.q2_track < 22)                     ]

    ax2.hist(df_smr[Data.bmass], bins=60, range=Data.mass_rng, label='$q^2$'        , alpha   =   0.2, color='blue' )
    ax2.hist(df_dtf[Data.bmass], bins=60, range=Data.mass_rng, label='$q^2_{DTF}$'  , histtype='step', color='green')
    ax2.hist(df_trk[Data.bmass], bins=60, range=Data.mass_rng, label='$q^2_{track}$', histtype='step', color='red'  )

    arr_q2smr = df[(df.q2_smr   < 22)                     ]['q2_smr'  ].to_numpy()
    arr_q2dtf = df[(df.q2_dtf   < 22) & (df.q2_smr   < 22)]['q2_dtf'  ].to_numpy()
    arr_q2trk = df[(df.q2_track < 22)                     ]['q2_track'].to_numpy()

    ax3= ax1.twinx()
    ax1.set_ylim(0, 0.20)
    ax3.set_ylim(0, 0.40)
    ax3.set_xlim(Data.min_q2, Data.max_q2)

    _plot_eff(arr_q2smr, color='blue' , ax=ax3)
    _plot_eff(arr_q2dtf, color='green', ax=ax3)
    _plot_eff(arr_q2trk, color='red'  , ax=ax3)

    ax3.tick_params(axis='y', labelcolor='green')
    ax3.set_ylabel('Efficiency', color='green')
    ax1.set_xlabel('$q^2$[GeV$/c^{2}$]')
    ax2.set_xlabel('$M(B^+)$[MeV$/c^{2}$]')

    ax1.legend(loc='upper right', bbox_to_anchor=(0.9, 0.9))
    ax2.legend(loc='upper right', bbox_to_anchor=(0.9, 0.9))

    title = _get_title(brem=brem)

    fig.suptitle(title, fontsize=40)
    ax1.axvline(x=15  , c='black', ls=':')
    ax2.axvline(x=5280, c='black', ls=':')

    plt.grid()
    plt.savefig(f'{Data.plt_dir}/q2_{brem}.png')
    plt.close()
# ---------------------------
def _get_title(brem : str) -> str:
    latex = Data.d_latex[Data.sample]

    return f'Brem = {brem}; {latex}'
# ---------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    rdf = _get_rdf()

    _plot(rdf=rdf)
# ---------------------------
if __name__ == '__main__':
    main()
