from data_checks.ntuple_filter import ntuple_filter
from log_store                 import log_store

#---------------------------------------
def set_log():
    log_store.set_level('data_checks:ntuple_filter', 10)
    log_store.set_level('rx_scripts:atr_mgr:mgr'   , 30)
#---------------------------------------
def test_dt():
    obj=ntuple_filter(dataset='dt_2024_turbo', cfg_ver='comp', index=1, ngroup=100)
    obj.filter()
#---------------------------------------
def test_mc():
    obj=ntuple_filter(dataset='mc_2024_turbo', cfg_ver='comp', index=1, ngroup=10)
    obj.filter()
#---------------------------------------
def  main():
    set_log()
    test_mc()
    test_dt()
#---------------------------------------
if __name__  == '__main__':
    main()

