'''
Script used to plot PID distributions from:

    - Samples with PID removed and original ones
    - Check that the distributions agree when the former ones have had the PID added
'''

import os
import argparse

from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_plots:validate_nopid')
# ----------------------------
class Data:
    '''
    Data class
    '''
    channel : str
    ana_dir = os.environ['ANADIR']
    fname   = '00012345_00006789_2.tuple.root'
    tname   = 'Hlt2RD_BuToKpEE_MVA'
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-c', '--channel', type=str, help='Channel where the validation will be ran', choices=['ee', 'mm'], required=True)
    args = parser.parse_args()

    Data.channel = args.channel
# ----------------------------
def _get_rdf(has_pid : bool) -> RDataFrame:
    root_path = f'{Data.ana_dir}/ana_prod/ntuples/no_pid/{Data.channel}/{Data.fname}'

    if has_pid:
        tname = f'{Data.tname}_noPID'
    else:
        tname = f'{Data.tname}'

    rdf = RDataFrame(f'{tname}/DecayTree', root_path)

    return rdf
# ----------------------------
def _compare(rdf_nopid : RDataFrame,
             rdf_yspid : RDataFrame) -> None:
    pass
# ----------------------------
def main():
    '''
    Start here
    '''
    _parse_args()

    rdf_nopid = _get_rdf(has_pid=False)
    rdf_yspid = _get_rdf(has_pid=True )

    _compare(
            rdf_nopid=rdf_nopid,
            rdf_yspid=rdf_yspid)
# ----------------------------
if __name__ == '__main__':
    main()
