from fitter    import ParameterReader
from rx_common import Component
from rx_common import Trigger 

def test_simple():
    '''
    Test simplest use of reader
    '''
    rdr    = ParameterReader()
    ms_sim = rdr(component=Component.jpsi, brem=1, block=3, trigger=Trigger.rk_ee_os)
    ms_dat = rdr(component=Component.data, brem=1, block=3, trigger=Trigger.rk_ee_os)
    
    _, _ = ms_sim.yield
    _, _ = ms_dat.yield
