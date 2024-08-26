from data_checks.ntuple_filter import ntuple_filter
from log_store                 import log_store

#---------------------------------------
def set_log():
    log_store.set_level('data_checks:ntuple_filter', 10)
#---------------------------------------
def test_simple():
    obj=ntuple_filter(cfg_nam='dt_2024_turbo', index=1, groups=100)
    obj.filter()
#---------------------------------------
def  main():
    set_log()
    test_simple()
#---------------------------------------
if __name__  == '__main__':
    main()
