'''
Script used to plot luminosity
'''

import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt
from ROOT import RDataFrame

plt.style.use(mplhep.style.LHCb2)
# ---------------------------------
class Data:
    '''
    Data class
    '''
    data_dir = '/home/acampove/PFS_data/lbpkmumu_run3/v1_data'

    d_dq : dict[int,int]
# ---------------------------------
def _add_error(df : pnd.DataFrame) -> pnd.DataFrame:
    return df
# ---------------------------------
def _get_df(name : str) -> pnd.DataFrame:
    file_path = f'/home/acampove/Downloads/{name}/*.root'
    print(f'Opening {file_path}')

    rdf =RDataFrame('t', file_path)
    data=rdf.AsNumpy()
    df  =pnd.DataFrame(data)
    df  =df.sort_values(by='runNumber')
    df  = _add_error(df)

    return df
# ---------------------------------
def _get_dataq(run_number : int) -> int:
    if run_number not in Data.d_dq:
        return -1

    return Data.d_dq[run_number]
# ---------------------------------
def _attach_dq(df : pnd.DataFrame) -> pnd.DataFrame:
    data_wc = f'{Data.data_dir}/*.root'
    print(f'Opening {data_wc}')

    rdf     = RDataFrame('DecayTree', data_wc)
    data    = rdf.AsNumpy(['RUNNUMBER', 'dataq'])
    df_dt   = pnd.DataFrame(data)
    df_dt   = df_dt.drop_duplicates()
    df_dt   = df_dt.set_index('RUNNUMBER')
    d_dq    = df_dt.dataq.to_dict()

    Data.d_dq   = d_dq
    df['dataq'] = df.runNumber.apply(_get_dataq)

    return df
# ---------------------------------
def _style_plot() -> None:
    plt.ylim(bottom=0)
    plt.xlabel('Run Number')
    plt.ylabel('Integrated Luminosity fb${}^{-1}$')
    plt.yscale('linear')
    plt.grid()
# ---------------------------------
def main():
    '''
    Start here
    '''
    df_rd_rap = _get_df(name = 'rd_ap_2024')
    df_lb_all = _get_df(name =     'lbpkmm')
    df_lb_all = _attach_dq(df_lb_all)

    df_lb_col = df_lb_all[df_lb_all.dataq != -1]
    df_lb_phy = df_lb_col[df_lb_col.dataq == +1]

    df_rd_rap['int_lumi'] = df_rd_rap.lumi.cumsum() / 1_000_000_000
    df_lb_col['int_lumi'] = df_lb_col.lumi.cumsum() / 1_000_000_000
    df_lb_phy['int_lumi'] = df_lb_phy.lumi.cumsum() / 1_000_000_000

    nrap = len(df_rd_rap)
    ncol = len(df_lb_col)
    nphy = len(df_lb_phy)

    print(f'RD  ap: {nrap}')
    print(f'Col Lb: {ncol}')
    print(f'Phy Lb: {nphy}')

    rd_lumi_rap = df_rd_rap.lumi.sum() / 1_000_000_000
    lb_lumi_col = df_lb_col.lumi.sum() / 1_000_000_000
    lb_lumi_phy = df_lb_phy.lumi.sum() / 1_000_000_000

    ax = None
    ax = df_rd_rap.plot(x='runNumber', y='int_lumi', color='black', label=f'{"RDWG":<10}{rd_lumi_rap:>4.2f} fb${{}}^{{-1}}$', ax=ax)
    ax = df_lb_col.plot(x='runNumber', y='int_lumi', color='red'  , label=f'{"Collected":<10}{lb_lumi_col:>4.2f} fb${{}}^{{-1}}$', ax=ax)
    ax = df_lb_phy.plot(x='runNumber', y='int_lumi', color='blue' , label=f'{"Good runs":<10}{lb_lumi_phy:>4.2f} fb${{}}^{{-1}}$', ax=ax)

    _style_plot()
    plt.show()

if __name__ == '__main__':
    main()
