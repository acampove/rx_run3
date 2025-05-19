'''
Script used to study Jpsi leakage in central q2 bin
'''
import os
import pandas as pnd

from rx_selection           import selection as sel
from rx_data.rdf_getter     import RDFGetter

from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_plot:leakage')
# --------------------------------
def _get_df() -> pnd.DataFrame:
    out_path = '/tmp/rx_plots/leakage/data.json'
    os.makedirs('/tmp/rx_plots/leakage', exist_ok=True)
    if os.path.isfile(out_path):
        log.info(f'Loading data from: {out_path}')
        return pnd.read_json(out_path)

    sample = 'Bu_JpsiK_ee_eq_DPC'
    trigger= 'Hlt2RD_BuToKpEE_MVA'

    gtr = RDFGetter(sample=sample, trigger=trigger)
    rdf = gtr.get_rdf()

    d_sel = sel.selection(trigger=trigger, q2bin='central', process=sample)
    d_sel['q2']   = '(1)'
    d_sel['mass'] = '(1)'

    for cut_name, cut_expr in d_sel.items():
        rdf = rdf.Filter(cut_expr, cut_name)

    columns = [
            'B_M_smr_brem_track_2',
            'Jpsi_M_smr_brem_track_2',
            'B_M_brem_track_2',
            'Jpsi_M_brem_track_2',
            ]

    data = rdf.AsNumpy(columns)
    df   = pnd.DataFrame(data)

    log.info(f'Saving data to: {out_path}')
    df.to_json(out_path, indent=4)

    return df
# --------------------------------
def main():
    '''
    Start here
    '''
    df = _get_df()
# --------------------------------
if __name__ == '__main__':
    main()

