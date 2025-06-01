'''
Script used to produce parquet files with the information needed by the
`PreProcessor` class of the `ecal_calibration` project
'''
import os
import pandas as pnd

from ROOT                   import RDataFrame
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_calibration:make_electron_calibration_data')
# --------------------------------
class Data:
    '''
    Data class
    '''
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    ana_dir = os.environ['ANADIR']

    l_branch = [
            'nPVs',
            'block',
            # ------------------
            'B_const_mass_M',
            'B_BPVX',
            'B_BPVY',
            'B_BPVZ',
            'B_END_VX',
            'B_END_VY',
            'B_END_VZ',
            # ------------------
            'H_PT' ,
            'H_ETA',
            'H_PHI',
            # ------------------
            'L1_PT_brem_track_2',
            'L1_ETA',
            'L1_PHI',
            'L1_brem',
            'L1_BREMHYPOROW',
            'L1_BREMHYPOCOL',
            'L1_BREMHYPOAREA',
            'L1_BREMTRACKBASEDENERGY',
            # ------------------
            'L2_PT_brem_track_2' ,
            'L2_ETA',
            'L2_PHI',
            'L2_brem',
            'L2_BREMHYPOROW',
            'L2_BREMHYPOCOL',
            'L2_BREMHYPOAREA',
            'L2_BREMTRACKBASEDENERGY']
# --------------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample='DATA*', trigger=Data.trigger)
    rdf = gtr.get_rdf()

    sel.set_custom_selection(d_cut = {
        'bdt'  : 'mva_cmb > 0.9 && mva_prc > 0.9',
        'tail' : 'B_const_mass_M > 5200 && B_const_mass_M < 5500'
        })

    d_sel = sel.selection(trigger=Data.trigger, q2bin='jpsi', process='DATA')

    for cut_name, cut_value in d_sel.items():
        rdf = rdf.Filter(cut_value, cut_name)

    rep = rdf.Report()
    rep.Print()

    return rdf
# --------------------------------
def _add_columns(rdf : RDataFrame) -> RDataFrame:
    rdf = rdf.Define('L1_brem', 'L1_HASBREMADDED_brem_track_2')
    rdf = rdf.Define('L2_brem', 'L2_HASBREMADDED_brem_track_2')

    return rdf
# --------------------------------
def _rename_columns(df : pnd.DataFrame) -> pnd.DataFrame:
    d_name = { name : name.replace('_brem_track_2', '') for name in df.columns }
    df     = df.rename(columns = d_name)

    return df
# --------------------------------
def _save_data(rdf : RDataFrame) -> None:
    out_dir = f'{Data.ana_dir}/Calibration/ecal'
    os.makedirs(out_dir, exist_ok=True)

    data = rdf.AsNumpy(Data.l_branch)
    df   = pnd.DataFrame(data)
    df   = _rename_columns(df=df)

    out_path = f'{out_dir}/data.parquet'
    log.info(f'Saving to: {out_path}')
    df.to_parquet(out_path)
# --------------------------------
def main():
    '''
    Start here
    '''
    rdf = _get_rdf()
    rdf = _add_columns(rdf=rdf)

    _save_data(rdf=rdf)
# --------------------------------
if __name__ == '__main__':
    main()
