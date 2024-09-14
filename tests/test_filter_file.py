from data_checks.filter_file import FilterFile
from log_store               import log_store

import utils_noroot          as utnr
import data_checks.utilities as ut


# --------------------------------------
class data:
    dt_path = '/home/acampove/data/aprod/raw/dt/00231366_00000013_1.ftuple.root'
    mc_path = '/home/acampove/data/aprod/raw/mc/BdKstmumu.root'
# --------------------------------------
def set_log():
    log_store.set_level('rx_scripts:atr_mgr:mgr', 30)
    log_store.set_level('data_checks:selector'  , 10)
    log_store.set_level('data_checks:utilities' , 30)
    log_store.set_level('data_checks:FilterFile', 10)
# --------------------------------------
def test_dt():
    obj = FilterFile(kind='any_kind', file_path=data.dt_path, cfg_nam='dt_2024_turbo_comp')
    obj.run()
# --------------------------------------
def test_mc():
    obj = FilterFile(kind='any_kind', file_path=data.mc_path, cfg_nam='mc_2024_turbo_comp')
    obj.run()
# --------------------------------------
def main():
    utnr.timer_on   = False
    ut.local_config = True

    set_log()
    test_mc()
    return
    test_dt()
# --------------------------------------


if __name__ == '__main__':
    main()
