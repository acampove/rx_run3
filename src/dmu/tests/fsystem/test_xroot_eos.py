'''
This module holds tests for functions in xroot_eos.py
'''

from dmu import XRootEOS 

def test_glob_eos():
    '''
    Tests globbing in EOS
    '''
    host = "root://eoslhcb.cern.ch"
    path = "/eos/lhcb/wg/dpa/wp2/ci/22781/btoxll_mva_2024_nopid"

    obj   = XRootEOS(host = host)
    paths = obj.glob(dir_path = path, ext = 'root')

