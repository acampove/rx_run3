from data_checks.filter_file import FilterFile
from log_store               import log_store

import utils_noroot as utnr
#--------------------------------------
class data:
    file_path='/home/acampove/data/aprod/raw/00231366_00000013_1.ftuple.root'
#--------------------------------------
def set_log():
    log_store.set_level('data_checks:FilterFile', 10)
    log_store.set_level('rx_scripts:atr_mgr:mgr', 30)
#--------------------------------------
def test_simple():
    obj=FilterFile(file_path=data.file_path, cfg_nam='dt_2024_turbo')
    obj.run()
#--------------------------------------
def main():
    utnr.timer_on=True

    set_log()
    test_simple()
#--------------------------------------
if __name__ == '__main__':
    main()

