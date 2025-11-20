from fitter    import ParameterReader
from rx_common import Project, Qsq
from rx_common import Trigger 

def test_simple():
    '''
    Test simplest use of reader
    '''
    rdr = ParameterReader(name = 'mid_window')
    ms_sim = rdr(
        brem     = 1, 
        block    = 3, 
        trigger  = Trigger.rk_ee_os, 
        project  = Project.rk,
        q2bin    = Qsq.jpsi)

    print(ms_sim)
