'''
Module with tests for FitParameters class 
'''
from rx_plotter import FitParameters
from dmu        import LogStore

log=LogStore.add_logger('rx_plotter:test_fit_parameters')
# ----------------------
def test_simple():
    fp = FitParameters(name = 'mid_window', cfg = 'fpars')
    fp.run()
