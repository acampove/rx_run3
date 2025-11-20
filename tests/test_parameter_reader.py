from fitter    import ParameterReader
from rx_common import Component, Qsq
from rx_common import Trigger 

def test_simple():
    '''
    Test simplest use of reader
    '''
    rdr    = ParameterReader(name = 'mid_window')
    ms_sim = rdr(component=Component.jpsi, brem=1, block=3, trigger=Trigger.rk_ee_os, q2bin = Qsq.jpsi)
    ms_dat = rdr(component=Component.data, brem=1, block=3, trigger=Trigger.rk_ee_os, q2bin = Qsq.jpsi)

    _, _ = ms_sim.candidates
    _, _ = ms_dat.candidates
