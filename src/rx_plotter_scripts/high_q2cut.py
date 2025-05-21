'''
Script used to study effect of different cuts on q2
to get signal in high q2 bin
'''
import numpy
import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                import RDataFrame
from rx_data.rdf_getter  import RDFGetter
from rx_selection        import selection as sel

# ---------------------------
class Data:
    '''
    Data class
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    sample  = 'Bu_Kee_eq_btosllball05_DPC'
    q2bin   = 'high'
    plt.style.use(mplhep.style.LHCb2)
    max_q2  = 22
    min_q2  = 14

    l_q2var = ['q2_smr', 'q2_track', 'nbrem']
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
    fig, ax1 = plt.subplots()

    df['q2_smr'  ].plot.hist(bins=60, range=[0, Data.max_q2], density=True, label='$q^2$'        , alpha = 0.2, color='blue')
    df['q2_track'].plot.hist(bins=60, range=[0, Data.max_q2], density=True, label='$q^2_{track}$', alpha = 0.2, color='red' )

    arr_q2smr = df['q2_smr'  ].to_numpy()
    arr_q2trk = df['q2_track'].to_numpy()

    ax2= ax1.twinx()
    ax1.set_ylim(0, 0.08)
    ax2.set_ylim(0, 0.25)
    ax2.set_xlim(Data.min_q2, Data.max_q2)

    _plot_eff(arr_q2smr, color='blue', ax=ax2)
    _plot_eff(arr_q2trk, color='red' , ax=ax2)

    ax2.tick_params(axis='y', labelcolor='green')
    ax2.set_ylabel('Efficiency', color='green')
    ax1.set_xlabel('$q^2$[GeV$/c^{2}$]')

    fig.legend(loc='upper right', bbox_to_anchor=(0.9, 0.9))
    plt.axvline(x=15, c='black', ls=':')
    plt.title(f'Brem {brem}')
    plt.grid()
    plt.savefig(f'q2_{brem}.png')
    plt.close()
# ---------------------------
def main():
    '''
    Start here
    '''
    rdf = _get_rdf()

    _plot(rdf=rdf)
# ---------------------------
if __name__ == '__main__':
    main()
