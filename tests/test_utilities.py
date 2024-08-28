import data_checks.utilities as ut
import pprint

import utils_noroot as utnr

from log_store import log_store

#----------------------------------------
def test_load_config():
    d_cfg = ut.load_config('dt_2024_turbo')

    pprint.pprint(d_cfg)
#----------------------------------------
def set_log():
    log_store.set_level('data_checks:utilities', 10)
#----------------------------------------
def main():
    utnr.timer_on=True
    set_log()

    test_load_config()
#----------------------------------------
if __name__ == '__main__':
    main()

