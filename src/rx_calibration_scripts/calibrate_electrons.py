'''
Script used to create calibration map for electrons
'''
import os
import dask.dataframe    as DaskDataFrame
import rx_data.utilities as rxut

from dmu.rfile.ddfgetter     import DDFGetter
from rx_calibration.electron.brem_energy import Calculator

# -------------------------------
class Data:
    '''
    Data class
    '''
    ana_dir = os.environ['ANADIR']
    sample  = 'DATA*'
    trigger = 'Hlt2RD_BuToKpEE_MVA'
    vers    = 'v1'
# -------------------------------
def _get_ddf() -> DaskDataFrame:
    cfg  = rxut.get_dask_config(sample=Data.sample, trigger=Data.trigger)
    ddfg = DDFGetter(cfg=cfg)
    ddf  = ddfg.get_dataframe()

    return ddf
# -------------------------------
def main():
    '''
    Start here
    '''
    ddf = _get_ddf()
    cal = Calculator(ddf=ddf)
    cal.train()
    cal.save(out_dir=f'{Data.ana_dir}/calibration/electron/brem_energy/{Data.vers}')
# -------------------------------
if __name__ == '__main__':
    main()
