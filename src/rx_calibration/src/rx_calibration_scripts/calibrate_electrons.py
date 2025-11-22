'''
Script used to create calibration map for electrons
'''
import os
import dask.dataframe    as DaskDataFrame

from rx_data.config_loader               import ConfigLoader
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
    ldr  = ConfigLoader(sample=Data.sample, trigger=Data.trigger, tree='DecayTree')
    cfg  = ldr.get_dask_conf()

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
