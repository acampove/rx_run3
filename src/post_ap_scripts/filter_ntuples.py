'''
Script used to filter ntuples produced by AP
'''

import argparse

from dmu.logging.log_store  import LogStore
from post_ap.ntuple_filter  import NtupleFilter

log=LogStore.add_logger('dmu:post_ap_scripts:filter_ntuples')
#----------------------------------------
class Data:
    '''
    Class used to store shared data
    '''
    prod    : str
    samp    : str
    ngroup  : int
    gindex  : int
    log_lv  : int
#----------------------------------------
def _set_log():
    LogStore.set_level('dmu:rdataframe:atr_mgr',          30)
    LogStore.set_level('post_ap:FilterFile'    , Data.log_lv)
    LogStore.set_level('post_ap:ntuple_filter' , Data.log_lv)
#----------------------------------------
def _get_args():
    parser = argparse.ArgumentParser(description='Will produce a smaller ntuple from a large one, for a given group of files')
    parser.add_argument('-p', '--prod'   , type=str, required=True , help='Production name, e.g. rd_ap_2024')
    parser.add_argument('-s', '--samp'   , type=str, required=True , help='Sample nickname, e.g. data, simulation')
    parser.add_argument('-g', '--ngroup' , type=int, required=True , help='Number of groups of files')
    parser.add_argument('-i', '--gindex' , type=int, required=True , help='Index of the current group been processed')
    parser.add_argument('-l', '--loglvl' , type=int, required=False, help='Loglevel', default=20, choices=[10, 20, 30, 40])
    args = parser.parse_args()

    Data.prod   = args.prod
    Data.samp   = args.samp
    Data.ngroup = args.ngroup
    Data.gindex = args.gindex
    Data.log_lv = args.loglvl
#----------------------------------------
def main():
    '''
    Execution starts here
    '''
    _get_args()
    _set_log()

    obj=NtupleFilter(index=Data.gindex, ngroup=Data.ngroup)
    obj.filter()
#----------------------------------------
if __name__ == '__main__':
    main()
